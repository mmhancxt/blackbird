#ifndef KRAKEN_H
#define KRAKEN_H

#include "quote_t.h"
#include <string>

struct json_t;
struct Parameters;

namespace Kraken {

quote_t getQuote(const Parameters& params, const std::string& currencyPair);

double getAvail(const Parameters& params, std::string currency);

std::string sendLongOrder(const Parameters& params, std::string direction, double quantity, double price);

std::string sendShortOrder(const Parameters& params, std::string direction, double quantity, double price);

std::string sendOrder(const Parameters& params, std::string direction, double quantity, double price);

bool isOrderComplete(const Parameters& params, std::string orderId);

double getActivePos(const Parameters& params);

double getLimitPrice(const Parameters& params, double volume, bool isBid);

json_t* authRequest(const Parameters& params, std::string request, std::string options = "");

void testKraken();

}

#endif
