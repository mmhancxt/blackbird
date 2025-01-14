#include "Binance.h"
#include "parameters.h"
#include "utils/restapi.h"
#include "utils/StringUtil.h"
#include "unique_json.hpp"
#include "hex_str.hpp"
#include "time_fun.h"
#include "openssl/sha.h"
#include "openssl/hmac.h"
#include "Wallet.h"
#include <array>
#include <algorithm>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <boost/algorithm/string/case_conv.hpp>

static json_t *authRequest(const Parameters &, std::string, std::string, std::string);

static std::string getSignature(const Parameters &params, std::string payload);

static RestApi &queryHandle(const Parameters &params)
{
   static RestApi query("https://api.binance.com",
         params.cacert.c_str(), params.logger);
   return query;
}


static std::unordered_map<std::string, std::string> s_binanceSpecialNameMap =
{
   {"REP", "REPV2"}
};


bool Binance::RetrieveInstruments()
{
   auto &exchange = queryHandle(m_params);
   std::string x = "/api/v3/exchangeInfo";
   unique_json root{exchange.getRequest(x)};

   json_t *resultArray = json_object_get(root.get(), "symbols");

   size_t index;
   json_t *symbolInfo = nullptr;
   json_array_foreach(resultArray, index, symbolInfo)
   {
      std::string symbol = json_string_value(json_object_get(symbolInfo, "symbol"));
      // m_log << "DEBUG : binance symbol is " << symbol << std::endl;
      const std::string status = json_string_value(json_object_get(symbolInfo, "status"));
      if (status != "TRADING")
      {
         m_log->info("Binace: {} trading phase is not TRADING[{}], skip", symbol, status);
         continue;
      }

      const std::string baseCcy = json_string_value(json_object_get(symbolInfo, "baseAsset"));
      const std::string quoteCcy = json_string_value(json_object_get(symbolInfo, "quoteAsset"));

      auto it = s_binanceSpecialNameMap.find(baseCcy);
      if (it != s_binanceSpecialNameMap.end())
      {
         utils::Replace(symbol, baseCcy, it->second);
      }
      it = s_binanceSpecialNameMap.find(quoteCcy);
      if (it != s_binanceSpecialNameMap.end())
      {
         utils::Replace(symbol, quoteCcy, it->second);
      }

      auto wsName(symbol);
      boost::to_lower(wsName);
      Instrument *instrument = new Instrument(m_id, m_name, symbol, wsName, baseCcy, quoteCcy, m_fees, m_canShort);
      m_dico.AddInstrument(symbol, instrument);
   }
   m_log->info("Binance dico complete");
   return true;
}

bool Binance::InitializeWallet()
{
   m_wallet.reset(new Wallet(m_name, m_log));

   unique_json root{authRequest(m_params, "GET", "/api/v3/account", "")};
   size_t arraySize = json_array_size(json_object_get(root.get(), "balances"));
   auto balances = json_object_get(root.get(), "balances");

   for (size_t i = 0; i < arraySize; i++)
   {
      const std::string asset = json_string_value(json_object_get(json_array_get(balances, i), "asset"));
      if (m_subscriptionAssets.find(asset) != m_subscriptionAssets.end())
      {
         const std::string freeStr = json_string_value(json_object_get(json_array_get(balances, i), "free"));
         const std::string lockedStr = json_string_value(json_object_get(json_array_get(balances, i), "locked"));

         double free = atof(freeStr.c_str());
         double locked = atof(lockedStr.c_str());

         m_wallet->InitializeBalance(asset, free, locked);
      }
   }

   m_wallet->WriteBalanceToFile();

   return true;
}

/*
   quote_t Binance::GetQuote(const std::string& currencyPair)
   {
   auto &exchange = queryHandle(m_params);
   std::string x = "/api/v3/ticker/bookTicker?symbol=" + currencyPair;
//params.leg2.c_str();
unique_json root{exchange.getRequest(x)};
double quote = atof(json_string_value(json_object_get(root.get(), "bidPrice")));
auto bidValue = quote ? quote : 0.0;
quote = atof(json_string_value(json_object_get(root.get(), "askPrice")));
auto askValue = quote ? quote : 0.0;

return std::make_pair(bidValue, askValue);
}*/

double Binance::GetAvail(std::string currency)
{
   std::string cur_str;
   //cur_str += "symbol=BTCUSDT";
   if (currency.compare("USD") == 0)
   {
      cur_str += "USDT";
   }
   else
   {
      cur_str += currency.c_str();
   }

   unique_json root{authRequest(m_params, "GET", "/api/v3/account", "")};
   size_t arraySize = json_array_size(json_object_get(root.get(), "balances"));
   double available = 0.0;
   const char *currstr;
   auto balances = json_object_get(root.get(), "balances");
   for (size_t i = 0; i < arraySize; i++)
   {
      std::string tmpCurrency = json_string_value(json_object_get(json_array_get(balances, i), "asset"));
      if (tmpCurrency.compare(cur_str.c_str()) == 0)
      {
         currstr = json_string_value(json_object_get(json_array_get(balances, i), "free"));
         if (currstr != NULL)
         {
            available = atof(currstr);
         }
         else
         {
            m_log->info("<binance> Error with currency string");
            available = 0.0;
         }
      }
   }
   return available;
}
//TODO: Currency String here
std::string Binance::SendLongOrder(std::string direction, double quantity, double price)
{
   if (direction.compare("buy") != 0 && direction.compare("sell") != 0)
   {
      m_log->info("<Binance> Error: Neither \"buy\" nor \"sell\" selected");
      return "0";
   }

   // *m_params.logFile << "<Binance> Trying to send a \"" << direction << "\" limit order: "
   //                 << std::setprecision(8) << quantity << " @ $"
   //                 << std::setprecision(8) << price << "...\n";
   std::string symbol = "BTCUSDT";
   std::transform(direction.begin(), direction.end(), direction.begin(), toupper);
   std::string type = "LIMIT";
   std::string tif = "GTC";
   std::string pricelimit = std::to_string(price);
   std::string volume = std::to_string(quantity);
   std::string options = "symbol=" + symbol + "&side=" + direction + "&type=" + type + "&timeInForce=" + tif + "&price=" + pricelimit + "&quantity=" + volume;
   unique_json root{authRequest(m_params, "POST", "/api/v3/order", options)};
   long txid = json_integer_value(json_object_get(root.get(), "orderId"));
   std::string order = std::to_string(txid);
   // *m_params.logFile << "<Binance> Done (transaction ID: " << order << ")\n"
   //                 << std::endl;
   return order;
}

//TODO: probably not necessary
std::string Binance::SendShortOrder(std::string direction, double quantity, double price)
{
   return "0";
}

bool Binance::IsOrderComplete(std::string orderId)
{
   unique_json root{authRequest(m_params, "GET", "/api/v3/openOrders", "")};
   size_t arraySize = json_array_size(root.get());
   bool complete = true;
   const char *idstr;
   for (size_t i = 0; i < arraySize; i++)
   {
      //SUGGEST: this is sort of messy
      long tmpInt = json_integer_value(json_object_get(json_array_get(root.get(), i), "orderId"));
      std::string tmpId = std::to_string(tmpInt);
      if (tmpId.compare(orderId.c_str()) == 0)
      {
         idstr = json_string_value(json_object_get(json_array_get(root.get(), i), "status"));
         // *m_params.logFile << "<Binance> Order still open (Status:" << idstr << ")" << std::endl;
         complete = false;
      }
   }
   return complete;
}
//TODO: Currency
double Binance::GetActivePos()
{
   return GetAvail("BTC");
}

double Binance::GetLimitPrice(double volume, bool isBid)
{
   auto &exchange = queryHandle(m_params);
   //TODO build a real URI string here
   unique_json root{exchange.getRequest("/api/v1/depth?symbol=BTCUSDT")};
   auto bidask = json_object_get(root.get(), isBid ? "bids" : "asks");
   // *m_params.logFile << "<Binance Looking for a limit price to fill "
   //                 << std::setprecision(8) << std::fabs(volume) << " Legx...\n";
   double tmpVol = 0.0;
   double p = 0.0;
   double v;
   int i = 0;
   while (tmpVol < std::fabs(volume) * m_params.orderBookFactor)
   {
      p = atof(json_string_value(json_array_get(json_array_get(bidask, i), 0)));
      v = atof(json_string_value(json_array_get(json_array_get(bidask, i), 1)));
      // *m_params.logFile << "<Binance> order book: "
      //                 << std::setprecision(8) << v << "@$"
      //                 << std::setprecision(8) << p << std::endl;
      tmpVol += v;
      i++;
   }
   return p;
}

json_t *authRequest(const Parameters &params, std::string method, std::string request, std::string options)
{
   //create timestamp Binance is annoying and requires their servertime
   auto &exchange = queryHandle(params);
   unique_json stamper{exchange.getRequest("/api/v3/time")};
   long stamp = json_integer_value(json_object_get(stamper.get(), "serverTime"));
   std::string timestamp = std::to_string(stamp);

   // create empty payload
   std::string payload;
   std::string uri;
   std::string sig;
   //our headers, might want to edit later to go into options check
   std::array<std::string, 1> headers{
      "X-MBX-APIKEY:" + params.binanceApi,
   };
   if (method.compare("POST") == 0)
   {
      payload += options + "&timestamp=" + timestamp;
      sig += getSignature(params, payload);
      uri += request + "?" + options + "&timestamp=" + timestamp + "&signature=" + sig;
      return exchange.postRequest(uri, make_slist(std::begin(headers), std::end(headers)));
   }
   else
   {
      if (options.empty())
      {
         payload += "timestamp=" + timestamp;
         sig += getSignature(params, payload);
         uri += request + "?timestamp=" + timestamp + "&signature=" + sig;
      }
      else
      {
         payload += options + "&timestamp=" + timestamp;
         sig += getSignature(params, payload);
         uri += request + "?" + options + "&timestamp=" + timestamp + "&signature=" + sig;
      }
      return exchange.getRequest(uri, make_slist(std::begin(headers), std::end(headers)));
   }
}

static std::string getSignature(const Parameters &params, std::string payload)
{
   uint8_t *hmac_digest = HMAC(EVP_sha256(),
         params.binanceSecret.c_str(), params.binanceSecret.size(),
         reinterpret_cast<const uint8_t *>(payload.data()), payload.size(),
         NULL, NULL);
   std::string api_sign_header = hex_str(hmac_digest, hmac_digest + SHA256_DIGEST_LENGTH);
   return api_sign_header;
}
