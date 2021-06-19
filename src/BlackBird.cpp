#include "BlackBird.h"
#include "Instrument.h"
#include "Dico.h"
#include "result.h"
#include "check_entry_exit.h"
#include "exchanges/Kraken.h"
#include "exchanges/Binance.h"
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
        m_log << "Demo mode: trades won't be generated\n"
                << std::endl;
    }

    InitializeMarkets();

    InitializeInstruments();

    std::cout << "Log file generated: " << m_logFileName << "\nBlackbird is running... (pid "
        << getpid() << ")\n" << std::endl;

    // Inits cURL connections
    m_params.curl = curl_easy_init();
    // Shows the spreads
    m_log << "[ Targets ]\n"
            << std::setprecision(2)
            << "   Spread Entry:  " << m_params.spreadEntry * 100.0 << "%\n"
            << "   Spread Target: " << m_params.spreadTarget * 100.0 << "%\n";

    // SpreadEntry and SpreadTarget have to be positive,
    // Otherwise we will loose money on every trade.
    if (m_params.spreadEntry <= 0.0)
    {
        m_log << "   WARNING: Spread Entry should be positive" << std::endl;
    }
    if (m_params.spreadTarget <= 0.0)
    {
        m_log << "   WARNING: Spread Target should be positive" << std::endl;
    }
    m_log << std::endl;
    m_log << "[ Current balances ]" << std::endl;
    // Gets the the balances from every exchange
    // This is only done when not in Demo mode.
    // TODO
    // std::vector<Balance> balance(numExch);
    // if (!m_params.isDemoMode)
    //     std::transform(getAvail, getAvail + numExch,
    //                    begin(balance),
    //                    [&m_params](decltype(*getAvail) apply)
    //                    {
    //                        Balance tmp{};
    //                        // TODO
    //                        // tmp.leg1 = apply(m_params, "btc");
    //                        // tmp.leg2 = apply(m_params, "usd");
    //                        return tmp;
    //                    });

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

    return true;
}

void BlackBird::InitializeMarkets()
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
    // We need at least two exchanges to run Blackbird
    if (index < 2)
    {
        std::cout << "ERROR: Blackbird needs at least two Bitcoin exchanges. Please edit the config.json file to add new exchanges\n"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

void BlackBird::InitializeInstruments()
{
    for (auto& p : m_markets)
    {
        auto& market = p.second;
        m_log << "Start to retrieve dico for " << p.first << std::endl;
        market->RetrieveInstruments();
        FilterCommonSymbols(market->GetDico());
    }

    m_log << "Common symbol size : " << m_commonSymbols.size() << std::endl;
    for (const auto& symbol : m_commonSymbols)
    {
      m_log << symbol << ",";
    }
    m_log << std::endl;
}

void BlackBird::FilterCommonSymbols(const Dico& dico)
{
  if (m_commonSymbols.empty())
  {
    for (const auto& p : dico.GetAllInstruments())
    {
      m_commonSymbols.insert(p.first);
    }
  }
  else
  {
    auto it = m_commonSymbols.begin();
    while (it != m_commonSymbols.end())
    {
      const auto& symbol = *it;
      if (dico.GetInstrumentBySymbol(symbol) == nullptr)
      {
        it = m_commonSymbols.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
}


void BlackBird::Run()
{
    // Code implementing the loop function, that runs
    // every 'Interval' seconds.
    time_t rawtime = time(nullptr);
    tm timeinfo = *localtime(&rawtime);
    using std::this_thread::sleep_for;
    using millisecs = std::chrono::milliseconds;
    using secs = std::chrono::seconds;
    // Waits for the next 'interval' seconds before starting the loop
    while ((int)timeinfo.tm_sec % m_params.interval != 0)
    {
        sleep_for(millisecs(100));
        time(&rawtime);
        timeinfo = *localtime(&rawtime);
    }
    if (!m_params.verbose)
    {
        m_log << "Running..." << std::endl;
    }

    LiveSource liveSource(m_params, m_markets, m_log);
    liveSource.Subscribe(m_commonSymbols);
    // liveSource.GetMarketData();
    // std::thread feedThread(&LiveSource::GetMarketData, &liveSource);

    int resultId = 0;
    unsigned currIteration = 0;
    bool stillRunning = true;
    time_t currTime;
    time_t diffTime;

    // Main analysis loop
    while (true)
    {
        /*
    currTime = mktime(&timeinfo);
    time(&rawtime);
    diffTime = difftime(rawtime, currTime);
    // Checks if we are already too late in the current iteration
    // If that's the case we wait until the next iteration
    // and we show a warning in the log file.
    if (diffTime > 0) {
      m_log << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
      timeinfo.tm_sec += (ceil(diffTime / m_params.interval) + 1) * m_params.interval;
      currTime = mktime(&timeinfo);
      sleep_for(secs(m_params.interval - (diffTime % m_params.interval)));
      m_log << std::endl;
    } else if (diffTime < 0) {
      sleep_for(secs(-diffTime));
    }
    // Header for every iteration of the loop
    if (m_params.verbose) {
      if (!inMarket) {
        m_log << "[ " << printDateTime(currTime) << " ]" << std::endl;
      } else {
        m_log << "[ " << printDateTime(currTime) << " IN MARKET: Long " << res.exchNameLong << " / Short " << res.exchNameShort << " ]" << std::endl;
      }
    }
    // Gets the bid and ask of all the exchanges
    for (const auto& currencyPair : m_params.tradedPair)
    {
      const auto& ccyPair = currencyPair.ToString();
      for (int i = 0; i < numExch; ++i) {
        auto quote = getQuote[i](m_params, ccyPair);
        double bid = quote.bid();
        double ask = quote.ask();

        // Saves the bid/ask into the SQLite database
        addBidAskToDb(dbTableName[i], ccyPair, printDateTimeDb(currTime), bid, ask, m_params);

        // If there is an error with the bid or ask (i.e. value is null),
        // we show a warning but we don't stop the loop.
        if (bid == 0.0) {
          m_log << "   WARNING: " << m_params.exchName[i] << " bid is null" << std::endl;
        }
        if (ask == 0.0) {
          m_log << "   WARNING: " << m_params.exchName[i] << " ask is null" << std::endl;
        }
        // Shows the bid/ask information in the log file
        if (m_params.verbose) {
          m_log << "   " << m_params.exchName[i] << ": \t"
                  << ccyPair << ": \t"
                  << std::setprecision(2)
                  << bid << " / " << ask << std::endl;
        }
        // Updates the Bitcoin vector with the latest bid/ask data
        auto it = exchangePairs[i].find(ccyPair);
        if (it != exchangePairs[i].end())
        {
          auto& ccyPairs = it->second;
          ccyPairs->safeUpdateData(quote);
          curl_easy_reset(m_params.curl);
        }
        else
        {
          m_log << "   ERROR: can't find ExchangePairs for " << ccyPair << " " << i << std::endl;
        }
      }
    }
    if (m_params.verbose) {
      m_log << "   ----------------------------" << std::endl;
    }
    */
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
        /* TODO
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
        timeinfo.tm_sec += m_params.interval;
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
