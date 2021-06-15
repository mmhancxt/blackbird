#include "time_fun.h"
#include "parameters.h"
#include <iostream>
#include <fstream>
#include "BlackBird.h"


// 'main' function.
// Blackbird doesn't require any arguments for now.
int main(int argc, char** argv) {
  std::cout << "Blackbird Bitcoin Arbitrage" << std::endl;
  std::cout << "DISCLAIMER: USE THE SOFTWARE AT YOUR OWN RISK\n" << std::endl;
  // Replaces the C++ global locale with the user-preferred locale
  // std::locale mylocale("");
  // Loads all the parameters
  Parameters params("blackbird.conf");
  // Does some verifications about the parameters
  if (!params.isDemoMode) {
    if (!params.useFullExposure) {
      if (params.testedExposure < 10.0 /*&& params.leg2.compare("USD") == 0*/) {
        // TODO do the same check for other currencies. Is there a limi?
        std::cout << "ERROR: Minimum USD needed: $10.00" << std::endl;
        std::cout << "       Otherwise some exchanges will reject the orders\n" << std::endl;
        exit(EXIT_FAILURE);
      }
      if (params.testedExposure > params.maxExposure) {
        std::cout << "ERROR: Test exposure (" << params.testedExposure << ") is above max exposure (" << params.maxExposure << ")\n" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }

  // Creates the CSV file that will collect the trade results
  std::string currDateTime = printDateTimeFileName();
  std::string csvFileName = "output/blackbird_result_" + currDateTime + ".csv";
  std::ofstream csvFile(csvFileName, std::ofstream::trunc);
  csvFile << "TRADE_ID,EXCHANGE_LONG,EXHANGE_SHORT,ENTRY_TIME,EXIT_TIME,DURATION,"
          << "TOTAL_EXPOSURE,BALANCE_BEFORE,BALANCE_AFTER,RETURN"
          << std::endl;
  // Creates the log file where all events will be saved
  std::string logFileName = "output/blackbird_log_" + currDateTime + ".log";
  std::ofstream logFile(logFileName, std::ofstream::trunc);
  logFile.imbue(std::locale());
  logFile.precision(2);
  logFile << std::fixed;
  params.logFile = &logFile;
  // Log file header
  logFile << "--------------------------------------------" << std::endl;
  logFile << "|   Blackbird Bitcoin Arbitrage Log File   |" << std::endl;
  logFile << "--------------------------------------------\n" << std::endl;
  logFile << "Blackbird started on " << printDateTime() << "\n" << std::endl;

  // logFile << "Connected to database \'" << params.dbFile << "\'\n" << std::endl;

  BlackBird blackBird(params, logFile, logFileName);
  blackBird.Initialize();
  blackBird.Run();

  csvFile.close();
  logFile.close();

  return 0;
}
