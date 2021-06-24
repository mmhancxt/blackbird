#include "time_fun.h"
#include "parameters.h"
#include <iostream>
#include <fstream>
#include "BlackBird.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"


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
  try
  {
    auto logger = spdlog::basic_logger_mt("basic_logger", logFileName, true);
    spdlog::flush_every(std::chrono::seconds(1));
    params.logger = logger;
    // Log file header
    logger->info("--------------------------------------------" );
    logger->info("|   Blackbird Bitcoin Arbitrage Log File   |");
    logger->info("--------------------------------------------");
    logger->info("Blackbird started on {}", printDateTime());

    // logFile << "Connected to database \'" << params.dbFile << "\'\n" << std::endl;

    BlackBird blackBird(params, logger, logFileName);
    blackBird.Initialize();
    blackBird.Run();
  }
  catch (const spdlog::spdlog_ex &ex)
  {
    std::cout << "Log init failed: " << ex.what() << std::endl;
  }

  csvFile.close();

  return 0;
}
