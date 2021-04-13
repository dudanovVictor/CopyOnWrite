/**
@file    ContractCache.cpp
@brief   Implementation of the ContractCache class.
@author  Victor Dudanov (victor.dudanov@gmail.com)
@version DEMO
*/

#include "ContractCache.h"
#include "ContractCacheImpl.h"

namespace mFIX
{
   // ---------------------------------------------------------------------------------------------
   ContractCache::ContractCache(TradingPlugin::IPluginContext& i_Context)
      : m_Impl(new Impl(i_Context))
   {
   }

   // ---------------------------------------------------------------------------------------------
   ContractCache::Result ContractCache::Get(TradingPlugin::ContractID i_ContractID,
      TradingPlugin::Symbology i_Symbology)
   {
      return m_Impl->Get(i_ContractID, i_Symbology);
   }

   // ---------------------------------------------------------------------------------------------
   void ContractCache::Update(TradingPlugin::ContractID i_ContractID, 
      TradingPlugin::Symbology i_Symbology, const TradingPlugin::ContractDesc* i_Desc)
   {
      m_Impl->Update(i_ContractID, i_Symbology, i_Desc);
   }

   // ---------------------------------------------------------------------------------------------
   ContractCache::CancelWatchFunc ContractCache::WatchForUpdates(const OnUpdateCallback& callback)
   {
      return m_Impl->WatchForUpdates(callback);
   }
} // mFIX namespace