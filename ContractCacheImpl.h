/**
@file    ContractCacheImpl.h
@brief   Declaration of the ContractCache::Impl class.
@author  Victor Dudanov (victor.dudanov@gmail.com)
@version DEMO
*/

#pragma once

#include "ContractCache.h"
#include <TradingPlugin/IPluginContext.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>

#include <list>
#include <map>
#include <utility>

namespace mFIX
{
/// @brief ContractCache::Impl class
class ContractCache::Impl
   : public boost::enable_shared_from_this<ContractCache::Impl>
   , private boost::noncopyable
{
public:
   /// @brief Ctor.
   /// @param i_Context Context provided by TradingPlugin host.
   explicit Impl(TradingPlugin::IPluginContext& i_Context);

   /// @name ContractCache methods
   /// @{
   Result Get(TradingPlugin::ContractID i_ContractID, TradingPlugin::Symbology i_Symbology);

   void Update(TradingPlugin::ContractID i_ContractID, TradingPlugin::Symbology i_Symbology,
      const TradingPlugin::ContractDesc* i_Desc);

   CancelWatchFunc WatchForUpdates(const OnUpdateCallback& callback);
   /// @}

private:
   /// @brief Creates and returns cancel watch functor
   /// @param i_IsSubscribed - reference to flag of subscription
   CancelWatchFunc createCancelWatchFunctor(bool& i_IsSubscribed);

   /// @brief Unsubscribes from updating
   /// @param i_ContractCache - a weak pointer to ContractCach instance
   /// @param i_IsSubscribed - reference to flag of subscription
   static void unsubscribe(boost::weak_ptr<ContractCache::Impl> i_ContractCache, bool& i_IsSubscribed);

   /// @brief Checks subscription and sends notification for all subscribers
   void notifyAll();

   /// @brief Checks and deletes dead subscribers
   /// @note Calls under lock
   void checkForDeadSubscribers();

   /// @brief Context provided by TradingPlugin host.
   TradingPlugin::IPluginContextPtr m_Context;

   ///@brief A callback entity
   struct CallbackEntity
   {
      /// @brief Callback for a cache update function
      OnUpdateCallback callback;

      /// @brief Subscribing flag
      bool isSubscribed;

      /// @brief Ctor
      explicit CallbackEntity(const OnUpdateCallback& i_Callback, bool i_IsSubscribed = true)
         : callback(i_Callback)
         , isSubscribed(i_IsSubscribed)
      {
      }
   };

   /// @brief Type of container with callbacks
   typedef std::list<CallbackEntity> UpdateCallbacks;

   /// @brief The container with callbacks
   UpdateCallbacks m_UpdateCallbacks;

   /// @brief Type of pointer to container of callbacks
   typedef boost::shared_ptr<UpdateCallbacks> UpdateCallbacksSPtr;

   /// @brief Mirror of the 'UpdateCallbacks'to achieve better scalability for MT scenario
   UpdateCallbacksSPtr m_UpdateCallbacksMirror;

   /// @brief Type of key for map of contract descriptions
   typedef std::pair<TradingPlugin::ContractID, TradingPlugin::Symbology> Key;

   /// @brief Type of a contract descriptions container
   typedef std::map<Key, ContractCache::Result> Contracts;

   /// @brief The contract descriptions container
   Contracts m_Contracts;

   /// @name Boost thread synchronization shortcuts.
   /// @{ 
   typedef boost::mutex mutex;
   typedef mutex::scoped_lock scoped_lock;
   /// @}

   /// @brief MT protection for container with contracts
   mutex m_ContractGuard;

   /// @brief MT protection for container with callbacks
   mutex m_CallbackGuard;
};
} // mFIX namespace