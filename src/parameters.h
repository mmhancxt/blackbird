#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "unique_sqlite.hpp"
#include "Instrument.h"

#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "spdlog/spdlog.h"

// Stores all the parameters defined
// in the configuration file.
struct Parameters {

  //std::vector<std::string> exchName;
  //std::vector<double> fees;
  //std::vector<bool> canShort;
  //std::vector<bool> isImplemented;
  //std::vector<std::string> tickerUrl;

  CURL* curl;
  double spreadEntry;
  double spreadTarget;
  unsigned maxLength;
  double priceDeltaLim;
  double trailingLim;
  unsigned trailingCount;
  double orderBookFactor;
  bool isDemoMode;
  std::vector<std::string> tradedPair;
  std::string bannedQuoteAsset;
  std::string tradedBaseAsset;
  std::string tradedQuoteAsset;
  unsigned int mdUpdateIntervalMs;
  //std::string leg1;
  //std::string leg2;
  bool verbose;
  //std::ofstream* logFile;
  std::shared_ptr<spdlog::logger> logger;
  unsigned interval;
  unsigned debugMaxIteration;
  bool useFullExposure;
  double testedExposure;
  double maxExposure;
  bool useVolatility;
  unsigned volatilityPeriod;
  std::string cacert;

  std::vector<std::string> indicatorList;

  std::string bitfinexApi;
  std::string bitfinexSecret;
  double bitfinexFees;
  bool bitfinexEnable;
  std::string okcoinApi;
  std::string okcoinSecret;
  double okcoinFees;
  bool okcoinEnable;
  std::string bitstampClientId;
  std::string bitstampApi;
  std::string bitstampSecret;
  double bitstampFees;
  bool bitstampEnable;
  std::string geminiApi;
  std::string geminiSecret;
  double geminiFees;
  bool geminiEnable;
  std::string krakenApi;

  std::string krakenSecret;
  double krakenFees;
  bool krakenEnable;
  bool krakenRequestMultiSymbols;

  std::string itbitApi;
  std::string itbitSecret;
  double itbitFees;
  bool itbitEnable;
  std::string wexApi;
  std::string wexSecret;
  double wexFees;
  bool wexEnable;
  std::string poloniexApi;
  std::string poloniexSecret;
  double poloniexFees;
  bool poloniexEnable;
  std::string gdaxApi;
  std::string gdaxSecret;
  std::string gdaxPhrase;
  double gdaxFees;
  bool gdaxEnable;
  std::string quadrigaApi;
  std::string quadrigaSecret;
  std::string quadrigaClientId;
  double quadrigaFees;
  bool quadrigaEnable;
  std::string exmoApi;
  std::string exmoSecret;
  double exmoFees;
  bool exmoEnable;
  std::string cexioClientId;
  std::string cexioApi;
  std::string cexioSecret;
  double cexioFees;
  bool cexioEnable;
  std::string bittrexApi;
  std::string bittrexSecret;
  double bittrexFees;
  bool bittrexEnable;

  std::string binanceApi;
  std::string binanceSecret;
  double binanceFees;
  bool binanceEnable;

  std::string coinbaseApi;
  std::string coinbaseSecret;
  double coinbaseFees;
  bool coinbaseEnable;

  bool sendEmail;
  std::string senderAddress;
  std::string senderUsername;
  std::string senderPassword;
  std::string smtpServerAddress;
  std::string receiverAddress;

  std::string dbFile;

  Parameters(std::string fileName);

  //void addExchange(std::string n, double f, bool h, bool m);

  //int nbExch() const;
};

// Copies the parameters from the configuration file
// to the Parameter structure.
std::string getParameter(std::string parameter, std::ifstream& configFile);

bool getBool(std::string value);

double getDouble(std::string value);

unsigned getUnsigned(std::string value);

#endif
