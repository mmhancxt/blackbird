#include "parameters.h"

#include <iostream>
#include <cstdlib>
#include <cassert>


static std::string findConfigFile(std::string fileName) {
  // local directory
  {
    std::ifstream configFile(fileName);

    // Keep the first match
    if (configFile.good()) {
      return fileName;
    }
  }

  // Unix user settings directory
  {
    char *home = getenv("HOME");

    if (home) {
      std::string prefix = std::string(home) + "/.config";
      std::string fullpath = prefix + "/" + fileName;
      std::ifstream configFile(fullpath);

      // Keep the first match
      if (configFile.good()) {
        return fullpath;
      }
    }
  }

  // Windows user settings directory
  {
    char *appdata = getenv("APPDATA");

    if (appdata) {
      std::string prefix = std::string(appdata);
      std::string fullpath = prefix + "/" + fileName;
      std::ifstream configFile(fullpath);

      // Keep the first match
      if (configFile.good()) {
        return fullpath;
      }
    }
  }

  // Unix system settings directory
  {
    std::string fullpath = "/etc/" + fileName;
    std::ifstream configFile(fullpath);

    // Keep the first match
    if (configFile.good()) {
      return fullpath;
    }
  }

  // We have to return something, even though we already know this will
  // fail
  return fileName;
}

void Split(const std::string& input, std::vector<std::string>& output, char delimiter)
{
  std::string s(input);
  size_t pos = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos)
  {
    token = s.substr(0, pos);
    output.push_back(token);
    s.erase(0, pos + 1);
  }
  output.push_back(s);
}

Parameters::Parameters(std::string fileName) {
  std::ifstream configFile(findConfigFile(fileName));
  if (!configFile.is_open()) {
    std::cout << "ERROR: " << fileName << " cannot be open.\n";
    exit(EXIT_FAILURE);
  }

  spreadEntry = getDouble(getParameter("SpreadEntry", configFile));
  spreadTarget = getDouble(getParameter("SpreadTarget", configFile));
  maxLength = getUnsigned(getParameter("MaxLength", configFile));
  priceDeltaLim = getDouble(getParameter("PriceDeltaLimit", configFile));
  trailingLim = getDouble(getParameter("TrailingSpreadLim", configFile));
  trailingCount = getUnsigned(getParameter("TrailingSpreadCount", configFile));
  orderBookFactor = getDouble(getParameter("OrderBookFactor", configFile));
  isDemoMode = getBool(getParameter("DemoMode", configFile));

  const std::string ccyList = getParameter("SpotCurrencyPairList", configFile);
  if (!ccyList.empty())
  {
    std::vector<std::string> pairs;
    Split(ccyList, pairs, ';');

    for (const auto& pair : pairs)
    {
      tradedPair.push_back(pair);
    }
  }

  bannedQuoteAsset = getParameter("BannedQuoteAsset", configFile);
  tradedBaseAsset = getParameter("TradedBaseAsset", configFile);
  tradedQuoteAsset = getParameter("TradedQuoteAsset", configFile);
  mdUpdateIntervalMs = getUnsigned(getParameter("MarketDataUpdateInvervalMs", configFile));

  //leg1 = getParameter("Leg1", configFile);
  //leg2 = getParameter("Leg2", configFile);
  verbose = getBool(getParameter("Verbose", configFile));
  interval = getUnsigned(getParameter("Interval", configFile));
  debugMaxIteration = getUnsigned(getParameter("DebugMaxIteration", configFile));
  useFullExposure = getBool(getParameter("UseFullExposure", configFile));
  testedExposure = getDouble(getParameter("TestedExposure", configFile));
  maxExposure = getDouble(getParameter("MaxExposure", configFile));
  useVolatility = getBool(getParameter("UseVolatility", configFile));
  volatilityPeriod = getUnsigned(getParameter("VolatilityPeriod", configFile));
  cacert = getParameter("CACert", configFile);

  const std::string indicList = getParameter("IndicatorList", configFile);
  if (!indicList.empty())
  {
    Split(indicList, indicatorList, ',');
  }

  bitfinexApi = getParameter("BitfinexApiKey", configFile);
  bitfinexSecret = getParameter("BitfinexSecretKey", configFile);
  bitfinexFees = getDouble(getParameter("BitfinexFees", configFile));
  bitfinexEnable = getBool(getParameter("BitfinexEnable", configFile));
  okcoinApi = getParameter("OkCoinApiKey", configFile);
  okcoinSecret = getParameter("OkCoinSecretKey", configFile);
  okcoinFees = getDouble(getParameter("OkCoinFees", configFile));
  okcoinEnable = getBool(getParameter("OkCoinEnable", configFile));
  bitstampClientId = getParameter("BitstampClientId", configFile);
  bitstampApi = getParameter("BitstampApiKey", configFile);
  bitstampSecret = getParameter("BitstampSecretKey", configFile);
  bitstampFees = getDouble(getParameter("BitstampFees", configFile));
  bitstampEnable = getBool(getParameter("BitstampEnable", configFile));
  geminiApi = getParameter("GeminiApiKey", configFile);
  geminiSecret = getParameter("GeminiSecretKey", configFile);
  geminiFees = getDouble(getParameter("GeminiFees", configFile));
  geminiEnable = getBool(getParameter("GeminiEnable", configFile));
  krakenApi = getParameter("KrakenApiKey", configFile);
  krakenSecret = getParameter("KrakenSecretKey", configFile);
  krakenFees = getDouble(getParameter("KrakenFees", configFile));
  krakenEnable = getBool(getParameter("KrakenEnable", configFile));
  krakenRequestMultiSymbols = getBool(getParameter("KrakenRequestMultiSymbols", configFile));
  itbitApi = getParameter("ItBitApiKey", configFile);
  itbitSecret = getParameter("ItBitSecretKey", configFile);
  itbitFees = getDouble(getParameter("ItBitFees", configFile));
  itbitEnable = getBool(getParameter("ItBitEnable", configFile));
  wexApi = getParameter("WEXApiKey", configFile);
  wexSecret = getParameter("WEXSecretKey", configFile);
  wexFees = getDouble(getParameter("WEXFees", configFile));
  wexEnable = getBool(getParameter("WEXEnable", configFile));
  poloniexApi = getParameter("PoloniexApiKey", configFile);
  poloniexSecret = getParameter("PoloniexSecretKey", configFile);
  poloniexFees = getDouble(getParameter("PoloniexFees", configFile));
  poloniexEnable = getBool(getParameter("PoloniexEnable", configFile));
  gdaxApi = getParameter("GDAXApiKey", configFile);
  gdaxSecret = getParameter("GDAXSecretKey", configFile);
  gdaxPhrase = getParameter("GDAXPhrase", configFile);
  gdaxFees = getDouble(getParameter("GDAXFees", configFile));
  gdaxEnable = getBool(getParameter("GDAXEnable", configFile));
  quadrigaApi = getParameter("QuadrigaApiKey", configFile);
  quadrigaSecret = getParameter("QuadrigaSecretKey", configFile);
  quadrigaFees = getDouble(getParameter("QuadrigaFees", configFile));
  quadrigaClientId = getParameter("QuadrigaClientId", configFile);
  quadrigaEnable = getBool(getParameter("QuadrigaEnable", configFile));
  exmoApi = getParameter("ExmoApiKey", configFile);
  exmoSecret = getParameter("ExmoSecretKey", configFile);
  exmoFees = getDouble(getParameter("ExmoFees", configFile));
  exmoEnable = getBool(getParameter("ExmoEnable", configFile));
  cexioClientId = getParameter("CexioClientId", configFile);
  cexioApi = getParameter("CexioApiKey", configFile);
  cexioSecret = getParameter("CexioSecretKey", configFile);
  cexioFees = getDouble(getParameter("CexioFees", configFile));
  cexioEnable = getBool(getParameter("CexioEnable", configFile));
  bittrexApi = getParameter("BittrexApiKey", configFile);
  bittrexSecret = getParameter("BittrexSecretKey", configFile);
  bittrexFees = getDouble(getParameter("BittrexFees", configFile));
  bittrexEnable = getBool(getParameter("BittrexEnable", configFile));

  binanceApi = getParameter("BinanceApiKey", configFile);
  binanceSecret = getParameter("BinanceSecretKey", configFile);
  binanceFees = getDouble(getParameter("BinanceFees", configFile));
  binanceEnable = getBool(getParameter("BinanceEnable", configFile));

  coinbaseApi = getParameter("CoinbaseApiKey", configFile);
  coinbaseSecret = getParameter("CoinbaseSecretKey", configFile);
  coinbaseFees = getDouble(getParameter("CoinbaseFees", configFile));
  coinbaseEnable = getBool(getParameter("CoinbaseEnable", configFile));

  sendEmail = getBool(getParameter("SendEmail", configFile));
  senderAddress = getParameter("SenderAddress", configFile);
  senderUsername = getParameter("SenderUsername", configFile);
  senderPassword = getParameter("SenderPassword", configFile);
  smtpServerAddress = getParameter("SmtpServerAddress", configFile);
  receiverAddress = getParameter("ReceiverAddress", configFile);

  dbFile = getParameter("DBFile", configFile);
}

//void Parameters::addExchange(std::string n, double f, bool h, bool m) {
//  exchName.push_back(n);
//  fees.push_back(f);
//  canShort.push_back(h);
//  isImplemented.push_back(m);
//}
//
//int Parameters::nbExch() const {
//  return exchName.size();
//}

std::string getParameter(std::string parameter, std::ifstream& configFile) {
  assert (configFile);
  std::string line;
  configFile.clear();
  configFile.seekg(0);

  while (getline(configFile, line)) {
    if (line.length() > 0 && line.at(0) != '#') {
      std::string key = line.substr(0, line.find('='));
      std::string value = line.substr(line.find('=') + 1, line.length());
      if (key == parameter) {
        return value;
      }
    }
  }

  std::cout << "ERROR: parameter '" << parameter << "' not found. Your configuration file might be too old.\n" << std::endl;
  exit(EXIT_FAILURE);
}

bool getBool(std::string value) {
  return value == "true";
}

double getDouble(std::string value) {
  return atof(value.c_str());
}

unsigned getUnsigned(std::string value) {
  return atoi(value.c_str());
}
