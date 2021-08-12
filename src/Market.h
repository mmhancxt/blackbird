#pragma once

#include <string>
#include <set>
#include "parameters.h"
#include "Instrument.h"
#include "Dico.h"
#include "spdlog/spdlog.h"
#include "Wallet.h"

class Market
{
public:
   Market(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
       : m_name(name), m_id(id), m_fees(fees), m_canShort(canShort), m_params(params), m_log(params.logger)
   {
   }

   virtual ~Market() = default;

   const std::string& GetName() const { return m_name; }
   int GetID() const { return m_id; }
   double GetFees() const { return m_fees; }
   bool CanShort() const { return m_canShort; }
   void SetRequestMultiSymbols(bool val) { m_requestMultiSymbols = val; }
   bool SupportReuesetMultiSymbols() const { return m_requestMultiSymbols; }

   const Dico& GetDico() const { return m_dico; }
   Dico& GetDico() { return m_dico; }

   virtual bool RetrieveInstruments() = 0;

   virtual bool InitializeWallet() = 0;

   // virtual quote_t GetQuote (const std::string& currencyPair) = 0;
   // virtual bool GetQuotesForMultiSymbols(const std::vector<std::string>& ccyPairs,
   //     std::unordered_map<std::string, quote_t>& quotes)
   // {
   //     return false;
   // }
   virtual double GetAvail(std::string currency) = 0;
   virtual std::string SendLongOrder(std::string direction, double quantity, double price) = 0;
   virtual std::string SendShortOrder(std::string direction, double quantity, double price) = 0;
   virtual bool IsOrderComplete(std::string orderId) = 0;
   virtual double GetActivePos() = 0;
   virtual double GetLimitPrice(double volume, bool isBid) = 0;

   void AddSubscriptionSymbol(const std::string& symbol)
   {
      m_subscriptionSymbols.insert(symbol);
   }

   void AddSubscriptionAsset(const std::string& asset)
   {
      m_subscriptionAssets.insert(asset);
   }

   const std::set<std::string>& GetSubscriptionSymbols() const { return m_subscriptionSymbols; }

   Instrument* GetInstrumentBySymbol(const std::string& symbol) const
   {
      if (m_subscriptionSymbols.find(symbol) != m_subscriptionSymbols.end())
      {
         return m_dico.GetInstrumentBySymbol(symbol);
      }
      else
      {
         return nullptr;
      }
   }

protected:

   std::string m_name;
   int m_id { 0 };
   const Parameters& m_params;
   std::shared_ptr<spdlog::logger> m_log;
   bool m_canShort { true };
   double m_fees { 0 };
   bool m_requestMultiSymbols { false };
   Dico m_dico;
   std::unique_ptr<Wallet> m_wallet;
   std::set<std::string> m_subscriptionSymbols;
   std::set<std::string> m_subscriptionAssets;
};
