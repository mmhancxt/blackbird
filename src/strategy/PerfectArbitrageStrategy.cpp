#include "strategy/PerfectArbitrageStrategy.h"
#include "Market.h"
#include "Parameters.h"
#include "result.h"


PerfectArbitrageStrategy::PerfectArbitrageStrategy(Parameters& params, 
      std::shared_ptr<spdlog::logger> log, 
      const std::unordered_map<std::string, std::unique_ptr<Market>>& markets,
      const std::set<std::string>& symbols)   
   : m_params(params), m_log(log), m_markets(markets), m_symbols(symbols)
{
}

void PerfectArbitrageStrategy::Poll()
{
   static int resultId = 0;
   time_t currTime;
   time_t diffTime;
   for (const auto& symbol : m_symbols)
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
               auto iter = m_openOperations.find(symbol);
               if (iter != m_openOperations.end())
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
                     m_openOperations.erase(symbol);
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
                     m_openOperations[symbol] = res;

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
                     m_openOperations[symbol] = res;

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
}
