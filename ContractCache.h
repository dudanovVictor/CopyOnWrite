/**
@file    ContractCache.h
@brief   Declaration of the ContractCache class.
@author  Victor Dudanov (victor.dudanov@gmail.com)
@version DEMO
*/

#pragma once

#include <TradingPlugin/BasicTypes.h>

#include <xxx/SmartEnum.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

// Forward declarations
namespace TradingPlugin
{
   struct ContractDesc;
   class IPluginContext;
}

namespace mFIX
{
   /// @brief The class is a cache of ContractDesc structures indexed by (ContractID, Symbology) pair.
   ///        The class requests TradingPlugin host for missed entries (via RequestContractDesc method).
   ///        Also it notifies its subscribers when the cache is updated. The class is thread-safe.
   class ContractCache
   {
   public:
      /// @brief Possible status values of entry returned by Get() method
      BEGIN_SMART_ENUM_INT(Status)
         ENUM_ENTRY(InProgress, 0) ///< entry data aren’t ready yet.
         ENUM_ENTRY(Ok, 1)         ///< entry data are valid.
         ENUM_ENTRY(Error, 2)      ///< no valid ContractDesc exist for this (ContractID, Symbology) pair.
      END_SMART_ENUM()

      /// @brief The structure is returned by Get() method and combines status
      ///        of the entry and pointer to ContractDesc (if status is Ok).
      struct Result
      {
         Status status;                           ///< status of the entry
         const TradingPlugin::ContractDesc* desc; ///< pointer to contract description

         explicit Result(Status i_Status = Status::Error,
            const TradingPlugin::ContractDesc* i_Desc = 0)
            : status(i_Status)
            , desc(i_Desc)
         {
         }
      };

      /// @brief Type of callback to be used in WatchForUpdates method.
      typedef boost::function<void ()> OnUpdateCallback;

      /// @brief Type of function object returned by WatchForUpdates method.
      ///        Used for to cancel the subscription.
      typedef boost::function<void ()> CancelWatchFunc;

      /// @brief Ctor.
      /// @param i_Context Context provided by TradingPlugin host.
      ContractCache(TradingPlugin::IPluginContext& i_Context);

      /// @brief Request ContractDesc data for specified contract and symbology.
      ///        If requested data is missed from cache then TradingPlugin host is requested.
      /// @param i_ContractID GW identifier of the contract.
      /// @param i_Symbology  Symbology of requested ContractDesc.
      /// @return Status of the entry and ContractDesc data if status is OK.
      Result Get(TradingPlugin::ContractID i_ContractID, TradingPlugin::Symbology i_Symbology);

      /// @brief Update specified entry in the cache and notify watchers of the cache.
      /// @param i_ContractID GW identifier of the contract.
      /// @param i_Symbology  Symbology of requested ContractDesc.
      /// @param i_Desc       Pointer to the ContractDesc data. If NULL then the entry will get status ‘Error’.
      void Update(TradingPlugin::ContractID i_ContractID, TradingPlugin::Symbology i_Symbology, 
         const TradingPlugin::ContractDesc* i_Desc);

      /// @brief Subscribe to notifications about cache changes.
      CancelWatchFunc WatchForUpdates(const OnUpdateCallback& callback);

   private:
      /// Forward declaration for Impl type.
      class Impl;
      /// @brief Smart pointer to the implementation.
      boost::shared_ptr<Impl> m_Impl;
   };

} // mFIX namespace
