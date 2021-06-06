#include <string>
#include "parameters.h"
#include "quote_t.h"
#include "bitcoin.h"
#include <unordered_map>
#pragma once

class Market
{
public:
    Market(const std::string& name, const Parameters& params) : m_name(name), m_params(params)
    {}
    quote_t GetQuote (const std::string& currencyPair);
    double GetAvail(std::string currency);
    std::string SendOrder(std::string direction, double quantity, double price);
    bool IsOrderComplete(std::string orderId);
    double GetActivePos();
    double GetLimitPrice(double volume, bool isBid);
private:
    std::string m_name;
    const Parameters& m_params;
    int m_id;
};

using ExchangePair = std::unordered_map<std::string, Bitcoin*>;

// The 'typedef' declarations needed for the function arrays
// These functions contain everything needed to communicate with
// the exchanges, like getting the quotes or the active positions.
// Each function is implemented in the files located in the 'exchanges' folder.
typedef quote_t (*getQuoteType) (const Parameters& params, const std::string& currencyPair);
typedef double (*getAvailType) (const Parameters& params, std::string currency);
typedef std::string (*sendOrderType) (const Parameters& params, std::string direction, double quantity, double price);
typedef bool (*isOrderCompleteType) (const Parameters& params, std::string orderId);
typedef double (*getActivePosType) (const Parameters& params);
typedef double (*getLimitPriceType) (const Parameters& params, double volume, bool isBid);
