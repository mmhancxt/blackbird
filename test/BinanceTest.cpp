#include <gtest/gtest.h>
#include "exchanges/Binance.h"
#include "parameters.h"
#include "spdlog/sinks/stdout_color_sinks.h"


TEST(Binance, test_wallet) {
   Parameters params("/Users/Xavier/git/blackbird/blackbird.conf");
   auto logger = spdlog::stdout_color_mt("console");
   params.logger = logger;
   Binance binance("binance", 0, params.binanceFees, true, params);

   std::string orderId;

   std::cout << "Current balance BTC: " << binance.GetAvail("btc") << std::endl;
   std::cout << "Current balance USD: " << binance.GetAvail("usd")<< std::endl;
   std::cout << "Current balance ETH: " << binance.GetAvail("eth")<< std::endl;
   std::cout << "Current balance BNB: " << binance.GetAvail("bnb")<< std::endl;
   EXPECT_EQ(1,1);
   //std::cout << "current bid limit price for .04 units: " << getLimitPrice(params, 0.04, true) << std::endl;
   //std::cout << "Current ask limit price for 10 units: " << getLimitPrice(params, 10.0, false) << std::endl;
   //std::cout << "Sending buy order for 0.003603 BTC @ BID! - TXID: " << std::endl;
   //orderId = sendLongOrder(params, "buy", 0.003603, getLimitPrice(params,0.003603,true));
   //std::cout << orderId << std::endl;
   //std::cout << "Buy Order is complete: " << isOrderComplete(params, orderId) << std::endl;

   //std::cout << "Sending sell order for 0.003603 BTC @ 10000 USD - TXID: " << std::endl;
   //orderId = sendLongOrder(params, "sell", 0.003603, 10000);
   //std::cout << orderId << std::endl;
   //std::cout << "Sell order is complete: " << isOrderComplete(params, orderId) << std::endl;
   //std::cout << "Active Position: " << getActivePos(params,orderId.c_str());
}

int main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

