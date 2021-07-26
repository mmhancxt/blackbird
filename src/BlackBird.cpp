#include "BlackBird.h"
#include "Instrument.h"
#include "Dico.h"
#include "check_entry_exit.h"
#include "exchanges/Kraken.h"
#include "exchanges/Binance.h"
#include "exchanges/Coinbase.h"
#include "curl_fun.h"
#include "utils/send_email.h"
#include "getpid.h"
#include "Market.h"
#include "LiveSource.h"
#include "strategy/PerfectArbitrageStrategy.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <chrono>
#include <thread>
#include <curl/curl.h>

bool BlackBird::Initialize()
{
   if (m_params.isDemoMode)
   {
      m_log->info("Demo mode: trades won't be generated");
   }

   std::cout << "Log file generated: " << m_logFileName << "\nBlackbird is running... (pid "
      << getpid() << ")\n" << std::endl;

   if (!InitializeMarkets())
   {
      m_log->error("Failed to initialize markets");
      return false;
   }

   InitializeInstruments();

   // Inits cURL connections
   m_params.curl = curl_easy_init();
   // Shows the spreads
   m_log->info("[ Targets ]");
   m_log->info("   Spread Entry:  {}%", m_params.spreadEntry * 100.0);
   m_log->info("   Spread Target: {}%", m_params.spreadTarget * 100.0);

   // SpreadEntry and SpreadTarget have to be positive,
   // Otherwise we will loose money on every trade.
   if (m_params.spreadEntry <= 0.0)
   {
      m_log->warn("   Spread Entry should be positive");
   }
   if (m_params.spreadTarget <= 0.0)
   {
      m_log->warn("   Spread Target should be positive");
   }
   m_log->info("---");
   m_log->info("[ Current balances ]");

   // Gets the the balances from every exchange
   // This is only done when not in Demo mode.
   // TODO
   for (auto& p : m_markets)
   {
      const auto& marketName = p.first;
      auto& market = p.second;
      if (!market->InitializeWallet())
      {
         m_log->error("Failed to initialize wallet for {}", marketName);
      }
   }

   if (!InitializeStrategies())
   {
      m_log->error("Failed to initialize strategies");
      return false;
   }

   /*
   // Checks for a restore.txt file, to see if
   // the program exited with an open position.
   Result res;
   res.reset();
   m_inMarket = res.loadPartialResult("restore.txt");

   // Writes the current balances into the log file
   for (const auto& p : m_markets)
   {
   const auto& market = p.second;
   m_log << "   " << p.first << ":\t";
   if (m_params.isDemoMode)
   {
   m_log << "n/a (demo mode)" << std::endl;
   }
   else
   {
   // TODO
   // m_log << std::setprecision(2) << balance[i].leg2 << " " << m_params.leg2 << "\t"
   //         << std::setprecision(6) << balance[i].leg1 << " " << m_params.leg1 << std::endl;
   }
   // TODO
   // if (balance[i].leg1 > 0.0050 && !inMarket) { // FIXME: hard-coded number
   //   m_log << "ERROR: All " << m_params.leg1 << " accounts must be empty before starting Blackbird" << std::endl;
   //   exit(EXIT_FAILURE);
   // }
   }
   m_log << std::endl;
   m_log << "[ Cash exposure ]\n";
   if (m_params.isDemoMode)
   {
   m_log << "   No cash - Demo mode\n";
   }
   else
   {
   if (m_params.useFullExposure)
   {
   m_log << "   FULL exposure used!\n";
   }
   else
   {
   m_log << "   TEST exposure used\n   Value: "
   << std::setprecision(2) << m_params.testedExposure << '\n';
   }
   }
   m_log << std::endl;
   */

   return true;
}

bool BlackBird::InitializeMarkets()
{
   // Adds the exchange functions to the arrays for all the defined exchanges
   int index = 0;
   if (m_params.krakenEnable &&
         (m_params.krakenApi.empty() == false || m_params.isDemoMode))
   {
      std::unique_ptr<Market> kraken = std::make_unique<Kraken>("kraken", index,
            m_params.krakenFees, true, m_params);

      kraken->SetRequestMultiSymbols(m_params.krakenRequestMultiSymbols);
      m_markets["kraken"] = std::move(kraken);

      index++;
   }
   if (m_params.binanceEnable &&
         (m_params.binanceApi.empty() == false || m_params.isDemoMode))
   {
      std::unique_ptr<Market> binance = std::make_unique<Binance>("binance", index,
            m_params.binanceFees, true, m_params);
      m_markets["binance"] = std::move(binance);

      index++;
   }
   if (m_params.coinbaseEnable &&
         (m_params.coinbaseApi.empty() == false || m_params.isDemoMode))
   {
      std::unique_ptr<Market> coinbase = std::make_unique<Coinbase>("coinbase", index,
            m_params.coinbaseFees, true, m_params);
      m_markets["coinbase"] = std::move(coinbase);

      index++;
   }
   // We need at least two exchanges to run Blackbird
   if (index < 2)
   {
      m_log->error("Blackbird needs at least two Bitcoin exchanges. Please edit the config.json file to add new exchanges");
      return false;
   }
   return true;
}

void BlackBird::InitializeInstruments()
{
   for (auto& p : m_markets)
   {
      auto& market = p.second;
      m_log->info("Start to retrieve dico for {}", p.first);
      market->RetrieveInstruments();
   }

   for (auto it = m_markets.begin(); it != m_markets.end(); ++it)
   {
      auto& marketName = it->first;
      auto& market = it->second;
      const auto& dico = market->GetDico();
      for (auto& p : dico.GetAllInstruments())
      {
         const auto& symbol = p.first;
         Instrument* instr = p.second;

         if (instr->ShouldSubscribe() == false
            && instr->GetQuoteCurrency() != m_params.bannedQuoteAsset
            && (m_params.tradedBaseAsset == "*" || instr->GetBaseCurrency() == m_params.tradedBaseAsset)
            && (m_params.tradedQuoteAsset == "*" || instr->GetQuoteCurrency() == m_params.tradedQuoteAsset))
         {
            auto it2(it);
            ++it2;
            while (it2 != m_markets.end())
            {
               auto& market2 = it2->second;
               const auto& dico2 = market2->GetDico();
               Instrument* instr2 = dico2.GetInstrumentBySymbol(symbol);
               if (instr2 != nullptr)
               {
                  instr->SetShouldSubscribe();
                  instr2->SetShouldSubscribe();
               }
               ++it2;
            }
         }

         if (instr->ShouldSubscribe())
         {
            market->AddSubscriptionSymbol(symbol);
            market->AddSubscriptionAsset(instr->GetBaseCurrency());
            market->AddSubscriptionAsset(instr->GetQuoteCurrency());
            m_allSubscriptionSymbols.insert(symbol);
         }
      }

      const auto& subscriptionSymbols = market->GetSubscriptionSymbols();
      m_log->info("{} subscription symbols size {}", marketName, subscriptionSymbols.size());
      std::stringstream ss;
      for (const auto& symbol : subscriptionSymbols)
      {
         ss << symbol << ",";
      }
      m_log->info(ss.str());
   }
}

bool BlackBird::InitializeStrategies()
{
   m_strategy = new PerfectArbitrageStrategy(m_params, m_log, m_markets, m_allSubscriptionSymbols);
   return true;
}

void BlackBird::Run()
{
   if (!m_params.verbose)
   {
      m_log->info("Running...");
   }

   LiveSource liveSource(m_params, m_markets, m_log);
   liveSource.Subscribe();

   // Main analysis loop
   while (true)
   {
      m_strategy->Poll();
   }

   // Analysis loop exited, does some cleanup
   curl_easy_cleanup(m_params.curl);
}
