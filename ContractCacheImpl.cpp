/**
@file    ContractCacheImpl.h
@brief   Implementation of the ContractCache::Impl class.
@author  Victor Dudanov (victor.dudanov@gmail.com)
@version DEMO
*/

#include "ContractCacheImpl.h"
#include <TradingPlugin/ContractDesc.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>

namespace mFIX
{
// ------------------------------------------------------------------------------------------------
ContractCache::Impl::Impl(TradingPlugin::IPluginContext& i_Context)
   : m_Context(&i_Context)
{
}

// ------------------------------------------------------------------------------------------------
ContractCache::Result ContractCache::Impl::Get(TradingPlugin::ContractID i_ContractID, 
   TradingPlugin::Symbology i_Symbology)
{
   bool isFound = false;

   ContractCache::Result returnedResult;
   {
      scoped_lock lk(m_ContractGuard);

      const Contracts::const_iterator iterFind = m_Contracts.find(Key(i_ContractID, i_Symbology));

      if(iterFind != m_Contracts.end())
      {
         isFound = true;

         returnedResult = iterFind->second;
      }
      else
      {
         returnedResult = m_Contracts.insert(Contracts::value_type(Key(i_ContractID, i_Symbology),
            ContractCache::Result(ContractCache::Status::InProgress))).first->second;
      }
   }

   if(!isFound)
   {
      m_Context->RequestContractDesc(i_ContractID, i_Symbology);
   }

   return returnedResult;
}

// ------------------------------------------------------------------------------------------------
void ContractCache::Impl::Update(TradingPlugin::ContractID i_ContractID,
   TradingPlugin::Symbology i_Symbology, const TradingPlugin::ContractDesc* i_Desc)
{
   {
      scoped_lock lk(m_ContractGuard);

      m_Contracts[Key(i_ContractID, i_Symbology)] = ContractCache::Result(
         i_Desc ? ContractCache::Status::Ok : ContractCache::Status::Error, i_Desc);
   }

   notifyAll();
}

// ------------------------------------------------------------------------------------------------
ContractCache::CancelWatchFunc ContractCache::Impl::WatchForUpdates(const OnUpdateCallback& callback)
{
   scoped_lock lk(m_CallbackGuard);

   bool& isSubscribed = m_UpdateCallbacks.insert(
      m_UpdateCallbacks.end(), CallbackEntity(callback))->isSubscribed;

   boost::atomic_store(&m_UpdateCallbacksMirror,
      boost::make_shared<UpdateCallbacks>(m_UpdateCallbacks));

   return createCancelWatchFunctor(isSubscribed);
}

// ------------------------------------------------------------------------------------------------
void ContractCache::Impl::notifyAll()
{
   if(m_UpdateCallbacksMirror)
   {
      const UpdateCallbacksSPtr& callbacks = boost::atomic_load(&m_UpdateCallbacksMirror);
      BOOST_FOREACH(const UpdateCallbacks::value_type& entity, *callbacks)
      {
         if(entity.isSubscribed)
         {
            entity.callback();
         }
      }
   }
}

// ------------------------------------------------------------------------------------------------
void ContractCache::Impl::checkForDeadSubscribers()
{
   // Removes unsubscribed entities
   m_UpdateCallbacks.remove_if(boost::bind(&CallbackEntity::isSubscribed, _1) == false);
   
   boost::atomic_store(&m_UpdateCallbacksMirror,
      boost::make_shared<UpdateCallbacks>(m_UpdateCallbacks));
}

// ------------------------------------------------------------------------------------------------
ContractCache::CancelWatchFunc ContractCache::Impl::createCancelWatchFunctor(bool& i_IsSubscribed)
{
   return boost::bind(&ContractCache::Impl::unsubscribe,
      shared_from_this(), boost::ref(i_IsSubscribed));
}

// ------------------------------------------------------------------------------------------------
void ContractCache::Impl::unsubscribe(boost::weak_ptr<ContractCache::Impl> i_ContractCache,
                                      bool& i_IsSubscribed)
{
   boost::shared_ptr<ContractCache::Impl> contractCache = i_ContractCache.lock();
   if(contractCache)
   {
      scoped_lock lk(contractCache->m_CallbackGuard);

      i_IsSubscribed = false;

      contractCache->checkForDeadSubscribers();
   }
}
} // mFIX namespace
