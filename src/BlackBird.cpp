#include "BlackBird.h"
#include "Instrument.h"
#include "Dico.h"
#include "result.h"
#include "check_entry_exit.h"
#include "exchanges/Kraken.h"
#include "exchanges/Binance.h"
#include "exchanges/Coinbase.h"
#include "curl_fun.h"
#include "utils/send_email.h"
#include "getpid.h"
#include "Market.h"
#include "LiveSource.h"

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

void BlackBird::Run()
{
   if (!m_params.verbose)
   {
      m_log->info("Running...");
   }

   LiveSource liveSource(m_params, m_markets, m_log);
   liveSource.Subscribe();

   int resultId = 0;
   unsigned currIteration = 0;
   bool stillRunning = true;
   time_t currTime;
   time_t diffTime;

   std::unordered_map<std::string, Result> openOperations;
   // Main analysis loop
   while (true)
   {
      for (const auto& symbol : m_allSubscriptionSymbols)
      {
         for (auto it = m_markets.begin(); it != m_markets.end(); ++it)
         {
            const auto& marketName1 = it->first;
            const auto* market1 = it->second.get();
            const auto& dico1 = market1->GetDico();
            auto* instr1 = dico1.GetInstrumentBySymbol(symbol);
            if (instr1 == nullptr || instr1->ShouldSubscribe() == false)
            {
               continue;
            }
            if (instr1->HasMarketUpdate())
            {
               const auto limit1 = instr1->SafeGetBestLimit();
               const auto& bid1 = limit1.Bid;
               const auto& ask1 = limit1.Ask;

               const double bid1Price = bid1.Price;
               const double bid1Qty = bid1.Quantity;
               const double ask1Price = ask1.Price;
               const double ask1Qty = ask1.Quantity;

               if (instr1->IsInMarket())
               {
                  // m_log->info("DEBUG: openOperations size is {}", openOperations.size());
                  auto iter = openOperations.find(symbol);
                  if (iter != openOperations.end())
                  {
                     auto& res = iter->second;
                     size_t otherSide = 2;
                     if (marketName1 == res.ExchName[0])
                     {
                        assert(instr1 == res.Instruments[0]);
                        otherSide = 1;
                     }
                     else if (marketName1 == res.ExchName[1])
                     {
                        assert(instr1 == res.Instruments[1]);
                        otherSide = 0;
                     }
                     else
                     {
                        m_log->error("Instrument {} {} is in market but can't find result", symbol, marketName1);
                        assert(false);
                     }

                     auto* otherInstr = res.Instruments[otherSide];
                     const auto limit2 = otherInstr->SafeGetBestLimitReadOnly();

                     const auto& bid2 = limit2.Bid;
                     const auto& ask2 = limit2.Ask;

                     const double bid2Price = bid2.Price;
                     const double bid2Qty = bid2.Quantity;
                     const double ask2Price = ask2.Price;
                     const double ask2Qty = ask2.Quantity;

                     double spread = 100;
                     if (otherSide == 1)
                     {
                        spread = (bid1Price - ask2Price)/ask2Price;
                     }
                     else
                     {
                        spread = (bid2Price - ask1Price)/ask1Price;
                     }

                     if (spread < m_params.spreadTarget)
                     {
                        m_log->info("Exit market for {}, {}/{}", symbol, marketName1, res.ExchName[otherSide]);
                        instr1->SetInMarket(false);
                        otherInstr->SetInMarket(false);
                        openOperations.erase(symbol);
                        currTime = std::time(0);
                        res.ExitTime = currTime;
                        res.PriceOut[0] = otherSide ? bid1Price : bid2Price;
                        res.PriceOut[1] = otherSide ? ask2Price : ask1Price;
                        //res.printExitInfo(m_log);
                     }
                  }
                  else
                  {
                     m_log->error("Failed to find open operation {} {}", symbol, marketName1);
                  }
               }
               else
               {
                  auto it2 = it;
                  ++it2;
                  while (it2 != m_markets.end() && !instr1->IsInMarket())
                  {
                     const auto& marketName2 = it2->first;
                     const auto* market2 = it2->second.get();
                     const auto& dico2 = market2->GetDico();
                     auto* instr2 = dico2.GetInstrumentBySymbol(symbol);

                     if (instr2 == nullptr || instr2->ShouldSubscribe() == false || instr2->IsInMarket())
                     {
                        ++it2;
                        continue;
                     }

                     const auto limit2 = instr2->SafeGetBestLimitReadOnly();

                     const auto& bid2 = limit2.Bid;
                     const auto& ask2 = limit2.Ask;

                     const double bid2Price = bid2.Price;
                     const double bid2Qty = bid2.Quantity;
                     const double ask2Price = ask2.Price;
                     const double ask2Qty = ask2.Quantity;

                     time_t now = std::time(nullptr);

                     const double profit1 = bid1Price - ask2Price - bid1Price * instr1->GetFees() - ask2Price * instr2->GetFees();
                     const double profit2 = bid2Price - ask1Price - ask1Price * instr1->GetFees() - bid2Price * instr2->GetFees();
                     bool found = false;
                     Result res;
                     if (profit1 > 0 && ask2Price != 0)
                     {
                        std::stringstream ss;
                        ss << "Found opportunity for " << symbol << " : " << marketName1 << " : " << std::fixed << std::setprecision(8) << bid1Price << " / "
                           << marketName2 << " : " << ask2Price << " profit " << profit1 << " " << profit1/ask2Price * 10000 << " bps";
                        m_log->info(ss.str());
                        found = true;
                        res.Symbol = symbol;
                        res.SpreadIn = (bid1Price-ask2Price)/ask2Price;
                        res.Fees[0] = instr2->GetFees();
                        res.Fees[1] = instr1->GetFees();
                        res.ExchName[0] = marketName2;
                        res.ExchName[1] = marketName1;
                        res.PriceIn[0] = ask2Price;
                        res.PriceIn[1] = bid1Price;
                        res.Instruments[0] = instr2;
                        res.Instruments[1] = instr1;

                        instr1->SetInMarket(true);
                        instr2->SetInMarket(true);
                        openOperations[symbol] = res;

                        //m_log->info("DEBUG: after 1 found openOperations size is {}", openOperations.size());
                     }
                     else if (profit2 > 0 && ask1Price != 0)
                     {
                        std::stringstream ss;
                        ss << "Found opportunity for " << symbol << " : " << marketName2 << " : " << std::fixed << std::setprecision(8) << bid2Price << " / "
                           << marketName1 << " : " << ask1Price << " profit " << profit2 << " " << profit2/ask2Price * 10000 << " bps";
                        m_log->info(ss.str());
                        found = true;
                        res.Symbol = symbol;
                        res.SpreadIn = (bid2Price-ask1Price)/ask1Price;
                        res.Fees[0] = instr1->GetFees();
                        res.Fees[1] = instr2->GetFees();
                        res.ExchName[0] = marketName1;
                        res.ExchName[1] = marketName2;
                        res.PriceIn[0] = ask1Price;
                        res.PriceIn[1] = bid2Price;
                        res.Instruments[0] = instr1;
                        res.Instruments[1] = instr2;

                        instr1->SetInMarket(true);
                        instr2->SetInMarket(true);
                        openOperations[symbol] = res;

                        //m_log->info("DEBUG: after 2 found openOperations size is {}", openOperations.size());
                     }

                     if (found)
                     {
                        currTime = std::time(0);
                        res.id = resultId++;
                        res.EntryTime = currTime;
                        //res.printEntryInfo(m_log);
                     }
                     
                     ++it2;
                  }
               }

            }
         }
      }
      /*
      // Header for every iteration of the loop
      if (m_params.verbose) {
      if (!inMarket) {
      m_log << "[ " << printDateTime(currTime) << " ]" << std::endl;
      } else {
      m_log << "[ " << printDateTime(currTime) << " IN MARKET: Long " << res.exchNameLong << " / Short " << res.exchNameShort << " ]" << std::endl;
      }
      } */
      // Stores all the spreads in arrays to
      // compute the volatility. The volatility
      // is not used for the moment.
      // TODO
      // if (m_params.useVolatility) {
      //   for (int i = 0; i < numExch; ++i) {
      //     for (int j = 0; j < numExch; ++j) {
      //       if (i != j) {
      //         if (btcVec[j].getHasShort()) {
      //           double longMidPrice = btcVec[i].getMidPrice();
      //           double shortMidPrice = btcVec[j].getMidPrice();
      //           if (longMidPrice > 0.0 && shortMidPrice > 0.0) {
      //             if (res.volatility[i][j].size() >= m_params.volatilityPeriod) {
      //               res.volatility[i][j].pop_back();
      //             }
      //             res.volatility[i][j].push_front((longMidPrice - shortMidPrice) / longMidPrice);
      //           }
      //         }
      //       }
      //     }
      //   }
      // }
      // Looks for arbitrage opportunities on all the exchange combinations
      /*
         if (!inMarket) {
         for (int i = 0; i < numExch; ++i) {
         for (int j = 0; j < numExch; ++j) {
         if (i != j) {
         if (checkEntry(&btcVec[i], &btcVec[j], res, m_params)) {
      // An entry opportunity has been found!
      res.exposure = std::min(balance[res.idExchLong].leg2, balance[res.idExchShort].leg2);
      if (m_params.isDemoMode) {
      m_log << "INFO: Opportunity found but no trade will be generated (Demo mode)" << std::endl;
      break;
      }
      if (res.exposure == 0.0) {
      m_log << "WARNING: Opportunity found but no cash available. Trade canceled" << std::endl;
      break;
      }
      if (m_params.useFullExposure == false && res.exposure <= m_params.testedExposure) {
      m_log << "WARNING: Opportunity found but no enough cash. Need more than TEST cash (min. $"
      << std::setprecision(2) << m_params.testedExposure << "). Trade canceled" << std::endl;
      break;
      }
      if (m_params.useFullExposure) {
      // Removes 1% of the exposure to have
      // a little bit of margin.
      res.exposure -= 0.01 * res.exposure;
      if (res.exposure > m_params.maxExposure) {
      m_log << "WARNING: Opportunity found but exposure ("
      << std::setprecision(2)
      << res.exposure << ") above the limit\n"
      << "         Max exposure will be used instead (" << m_params.maxExposure << ")" << std::endl;
      res.exposure = m_params.maxExposure;
      }
      } else {
      res.exposure = m_params.testedExposure;
      }
      // Checks the volumes and, based on that, computes the limit prices
      // that will be sent to the exchanges
      double volumeLong = res.exposure / btcVec[res.idExchLong].getAsk();
      double volumeShort = res.exposure / btcVec[res.idExchShort].getBid();
      double limPriceLong = getLimitPrice[res.idExchLong](m_params, volumeLong, false);
      double limPriceShort = getLimitPrice[res.idExchShort](m_params, volumeShort, true);
      if (limPriceLong == 0.0 || limPriceShort == 0.0) {
      m_log << "WARNING: Opportunity found but error with the order books (limit price is null). Trade canceled\n";
      m_log.precision(2);
      m_log << "         Long limit price:  " << limPriceLong << std::endl;
      m_log << "         Short limit price: " << limPriceShort << std::endl;
      res.trailing[res.idExchLong][res.idExchShort] = -1.0;
      break;
      }
      if (limPriceLong - res.priceLongIn > m_params.priceDeltaLim || res.priceShortIn - limPriceShort > m_params.priceDeltaLim) {
      m_log << "WARNING: Opportunity found but not enough liquidity. Trade canceled\n";
      m_log.precision(2);
      m_log << "         Target long price:  " << res.priceLongIn << ", Real long price:  " << limPriceLong << std::endl;
      m_log << "         Target short price: " << res.priceShortIn << ", Real short price: " << limPriceShort << std::endl;
      res.trailing[res.idExchLong][res.idExchShort] = -1.0;
      break;
      }
      // We are in market now, meaning we have positions on leg1 (the hedged on)
      // We store the details of that first trade into the Result structure.
      inMarket = true;
      resultId++;
      res.id = resultId;
      res.entryTime = currTime;
      res.priceLongIn = limPriceLong;
      res.priceShortIn = limPriceShort;
      res.printEntryInfo(*m_params.m_log);
      res.maxSpread[res.idExchLong][res.idExchShort] = -1.0;
      res.minSpread[res.idExchLong][res.idExchShort] = 1.0;
      res.trailing[res.idExchLong][res.idExchShort] = 1.0;

      // Send the orders to the two exchanges
      auto longOrderId = sendLongOrder[res.idExchLong](m_params, "buy", volumeLong, limPriceLong);
      auto shortOrderId = sendShortOrder[res.idExchShort](m_params, "sell", volumeShort, limPriceShort);
      m_log << "Waiting for the two orders to be filled..." << std::endl;
      sleep_for(millisecs(5000));
      bool isLongOrderComplete = isOrderComplete[res.idExchLong](m_params, longOrderId);
      bool isShortOrderComplete = isOrderComplete[res.idExchShort](m_params, shortOrderId);
      // Loops until both orders are completed
      while (!isLongOrderComplete || !isShortOrderComplete) {
         sleep_for(millisecs(3000));
         if (!isLongOrderComplete) {
            m_log << "Long order on " << m_params.exchName[res.idExchLong] << " still open..." << std::endl;
            isLongOrderComplete = isOrderComplete[res.idExchLong](m_params, longOrderId);
         }
         if (!isShortOrderComplete) {
            m_log << "Short order on " << m_params.exchName[res.idExchShort] << " still open..." << std::endl;
            isShortOrderComplete = isOrderComplete[res.idExchShort](m_params, shortOrderId);
         }
      }
      // Both orders are now fully executed
      m_log << "Done" << std::endl;

      // Stores the partial result to file in case
      // the program exits before closing the position.
      res.savePartialResult("restore.txt");

      // Resets the order ids
      longOrderId  = "0";
      shortOrderId = "0";
      break;
   }
   }
   }
   if (inMarket) {
      break;
   }
   }
   if (m_params.verbose) {
      m_log << std::endl;
   }
   } else if (inMarket) {
      // We are in market and looking for an exit opportunity
      if (checkExit(&btcVec[res.idExchLong], &btcVec[res.idExchShort], res, m_params, currTime)) {
         // An exit opportunity has been found!
         // We check the current leg1 exposure
         std::vector<double> btcUsed(numExch);
         for (int i = 0; i < numExch; ++i) {
            btcUsed[i] = getActivePos[i](m_params);
         }
         // Checks the volumes and computes the limit prices that will be sent to the exchanges
         double volumeLong = btcUsed[res.idExchLong];
         double volumeShort = btcUsed[res.idExchShort];
         double limPriceLong = getLimitPrice[res.idExchLong](m_params, volumeLong, true);
         double limPriceShort = getLimitPrice[res.idExchShort](m_params, volumeShort, false);
         if (limPriceLong == 0.0 || limPriceShort == 0.0) {
            m_log << "WARNING: Opportunity found but error with the order books (limit price is null). Trade canceled\n";
            m_log.precision(2);
            m_log << "         Long limit price:  " << limPriceLong << std::endl;
            m_log << "         Short limit price: " << limPriceShort << std::endl;
            res.trailing[res.idExchLong][res.idExchShort] = 1.0;
         } else if (res.priceLongOut - limPriceLong > m_params.priceDeltaLim || limPriceShort - res.priceShortOut > m_params.priceDeltaLim) {
            m_log << "WARNING: Opportunity found but not enough liquidity. Trade canceled\n";
            m_log.precision(2);
            m_log << "         Target long price:  " << res.priceLongOut << ", Real long price:  " << limPriceLong << std::endl;
            m_log << "         Target short price: " << res.priceShortOut << ", Real short price: " << limPriceShort << std::endl;
            res.trailing[res.idExchLong][res.idExchShort] = 1.0;
         } else {
            res.exitTime = currTime;
            res.priceLongOut = limPriceLong;
            res.priceShortOut = limPriceShort;
            res.printExitInfo(*m_params.m_log);

            // TODO
            // m_log.precision(6);
            // m_log << m_params.leg1 << " exposure on " << m_params.exchName[res.idExchLong] << ": " << volumeLong << '\n'
            //         << m_params.leg1 << " exposure on " << m_params.exchName[res.idExchShort] << ": " << volumeShort << '\n'
            //         << std::endl;
            auto longOrderId = sendLongOrder[res.idExchLong](m_params, "sell", fabs(btcUsed[res.idExchLong]), limPriceLong);
            auto shortOrderId = sendShortOrder[res.idExchShort](m_params, "buy", fabs(btcUsed[res.idExchShort]), limPriceShort);
            m_log << "Waiting for the two orders to be filled..." << std::endl;
            sleep_for(millisecs(5000));
            bool isLongOrderComplete = isOrderComplete[res.idExchLong](m_params, longOrderId);
            bool isShortOrderComplete = isOrderComplete[res.idExchShort](m_params, shortOrderId);
            // Loops until both orders are completed
            while (!isLongOrderComplete || !isShortOrderComplete) {
               sleep_for(millisecs(3000));
               if (!isLongOrderComplete) {
                  m_log << "Long order on " << m_params.exchName[res.idExchLong] << " still open..." << std::endl;
                  isLongOrderComplete = isOrderComplete[res.idExchLong](m_params, longOrderId);
               }
               if (!isShortOrderComplete) {
                  m_log << "Short order on " << m_params.exchName[res.idExchShort] << " still open..." << std::endl;
                  isShortOrderComplete = isOrderComplete[res.idExchShort](m_params, shortOrderId);
               }
            }
            m_log << "Done\n" << std::endl;
            longOrderId  = "0";
            shortOrderId = "0";
            inMarket = false;
            for (int i = 0; i < numExch; ++i) {
               balance[i].leg2After = getAvail[i](m_params, "usd"); // FIXME: currency hard-coded
               balance[i].leg1After = getAvail[i](m_params, "btc"); // FIXME: currency hard-coded
            }
            for (int i = 0; i < numExch; ++i) {
               m_log << "New balance on " << m_params.exchName[i] << ":  \t";
               m_log.precision(2);
               m_log << balance[i].leg2After << " " << m_params.leg2 << " (perf " << balance[i].leg2After - balance[i].leg2 << "), ";
               m_log << std::setprecision(6) << balance[i].leg1After << " " << m_params.leg1 << "\n";
            }
            m_log << std::endl;
            // Update total leg2 balance
            for (int i = 0; i < numExch; ++i) {
               res.leg2TotBalanceBefore += balance[i].leg2;
               res.leg2TotBalanceAfter  += balance[i].leg2After;
            }
            // Update current balances
            for (int i = 0; i < numExch; ++i) {
               balance[i].leg2 = balance[i].leg2After;
               balance[i].leg1 = balance[i].leg1After;
            }
            // Prints the result in the result CSV file
            m_log.precision(2);
            m_log << "ACTUAL PERFORMANCE: " << "$" << res.leg2TotBalanceAfter - res.leg2TotBalanceBefore << " (" << res.actualPerf() * 100.0 << "%)\n" << std::endl;
            csvFile << res.id << ","
               << res.exchNameLong << ","
               << res.exchNameShort << ","
               << printDateTimeCsv(res.entryTime) << ","
               << printDateTimeCsv(res.exitTime) << ","
               << res.getTradeLengthInMinute() << ","
               << res.exposure * 2.0 << ","
               << res.leg2TotBalanceBefore << ","
               << res.leg2TotBalanceAfter << ","
               << res.actualPerf() << std::endl;
            // Sends an email with the result of the trade
            if (m_params.sendEmail) {
               sendEmail(res, m_params);
               m_log << "Email sent" << std::endl;
            }
            res.reset();
            // Removes restore.txt since this trade is done.
            std::ofstream resFile("restore.txt", std::ofstream::trunc);
            resFile.close();
         }
      }
      if (m_params.verbose) m_log << '\n';
   }
   */
      // Moves to the next iteration, unless
      // the maxmum is reached.
      //timeinfo.tm_sec += m_params.interval;
      currIteration++;
   // if (currIteration >= m_params.debugMaxIteration)
   // {
   //     m_log << "Max iteration reached (" << m_params.debugMaxIteration << ")" << std::endl;
   //     stillRunning = false;
   // }
   // Exits if a 'stop_after_notrade' file is found
   // Warning: by default on GitHub the file has a underscore
   // at the end, so Blackbird is not stopped by default.
   // std::ifstream infile("stop_after_notrade");
   // if (infile && !m_inMarket)
   // {
   //     m_log << "Exit after last trade (file stop_after_notrade found)\n";
   //     stillRunning = false;
   // }
   }

   //feedThread.join();
   // Analysis loop exited, does some cleanup
   curl_easy_cleanup(m_params.curl);
}
