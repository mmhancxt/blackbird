#include "result.h"
#include "time_fun.h"
#include <type_traits>
#include <iostream>
#include <fstream>


double Result::targetPerfLong() const {
  return (PriceOut[0] - PriceIn[0]) / PriceIn[0] - 2.0 * Fees[0];
}

double Result::targetPerfShort() const {
  return (PriceIn[1] - PriceOut[1]) / PriceIn[1] - 2.0 * Fees[1];
}

double Result::actualPerf() const {
  if (exposure == 0.0) return 0.0;
  // The performance is computed as an "actual" performance,
  // by simply comparing what amount was on our leg2 account
  // before, and after, the arbitrage opportunity. This way,
  // we are sure that every fees was taking out of the performance
  // computation. Hence the "actual" performance.
  return (leg2TotBalanceAfter - leg2TotBalanceBefore) / (exposure * 2.0);
}

double Result::getTradeLengthInMinute() const {
  if (EntryTime > 0 && ExitTime > 0) return (ExitTime - EntryTime) / 60.0;
  return 0;
}

void Result::printEntryInfo(std::shared_ptr<spdlog::logger> logFile) const {
  logFile->info("\n[ ENTRY FOUND ]");
  logFile->info("   Date & Time:       {}", printDateTime(EntryTime));
  logFile->info("   Exchange Long:     {}", ExchName[0]);
  logFile->info("   Exchange Short:    {}", ExchName[1]);
  logFile->info("   Fees:              {}% / {}%", Fees[0] * 100.0, Fees[1] * 100.0);
  logFile->info("   Price Long:        {}", PriceIn[0]);
  logFile->info("   Price Short:       {}", PriceIn[1]);
  logFile->info("   Spread:            {}%", SpreadIn * 100.0);
  logFile->info("   Cash used:         {} on each exchange", exposure);
  logFile->info("   Exit Target:       {}%\n", ExitTarget * 100.0);
}

void Result::printExitInfo(std::shared_ptr<spdlog::logger> logFile) const {
  logFile->info("\n[ EXIT FOUND ]");
  logFile->info("   Date & Time:       {}", printDateTime(ExitTime));
  logFile->info("   Duration:          {} minutes", getTradeLengthInMinute());
  logFile->info("   Price Long:        {}", PriceOut[0]);
  logFile->info("   Price Short:       {}", PriceOut[1]);
  logFile->info("   Spread:            {}%", SpreadOut * 100.0);
  logFile->info("   ---------------------------");
  logFile->info("   Target Perf Long:  {}% (fees incl.)", targetPerfLong()  * 100.0);
  logFile->info("   Target Perf Short: {}% (fees incl.)", targetPerfShort() * 100.0);
  logFile->info("   ---------------------------\n");
}

// not sure to understand how this function is implemented ;-)
void Result::reset() {
   /*
  typedef std::remove_reference< decltype(minSpread[0][0]) >::type arr2d_t;
  auto arr2d_size = sizeof(minSpread) / sizeof(arr2d_t);
  Result tmp {};
  std::swap(tmp, *this);
  // Resets all the values of min, max and trailing arrays to values
  // that will be erased by the very first value entered in the respective arrays.
  // That's why the reset value for min is 1.0 and for max is -1.0.
  std::fill_n(reinterpret_cast<arr2d_t *>(minSpread), arr2d_size, 1.0);
  std::fill_n(reinterpret_cast<arr2d_t *>(maxSpread), arr2d_size, -1.0);
  std::fill_n(reinterpret_cast<arr2d_t *>(trailing), arr2d_size, -1.0);
  */
}

bool Result::loadPartialResult(std::string filename) {

   /*
  std::ifstream resFile(filename, std::ifstream::ate);
  if(!resFile || int(resFile.tellg()) == 0) return false;

  resFile.seekg(0);
  resFile >> id
          >> idExchLong   >> idExchShort
          >> exchNameLong >> exchNameShort
          >> exposure
          >> feesLong
          >> feesShort
          >> entryTime
          >> spreadIn
          >> priceLongIn
          >> priceShortIn
          >> leg2TotBalanceBefore
          >> exitTarget;

  resFile >> maxSpread[idExchLong][idExchShort]
          >> minSpread[idExchLong][idExchShort]
          >> trailing[idExchLong][idExchShort]
          >> trailingWaitCount[idExchLong][idExchShort];
          */

  return true;
}

void Result::savePartialResult(std::string filename) {
   /*
  std::ofstream resFile(filename, std::ofstream::trunc);

  resFile << id << '\n'
          << idExchLong << '\n'
          << idExchShort << '\n'
          << exchNameLong << '\n'
          << exchNameShort << '\n'
          << exposure << '\n'
          << feesLong << '\n'
          << feesShort << '\n'
          << entryTime << '\n'
          << spreadIn << '\n'
          << priceLongIn << '\n'
          << priceShortIn << '\n'
          << leg2TotBalanceBefore << '\n'
          << exitTarget << '\n';

  resFile << maxSpread[idExchLong][idExchShort] << '\n'
          << minSpread[idExchLong][idExchShort] << '\n'
          << trailing[idExchLong][idExchShort] << '\n'
          << trailingWaitCount[idExchLong][idExchShort]
          << std::endl;
          */
}
