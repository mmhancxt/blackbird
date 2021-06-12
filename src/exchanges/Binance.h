#pragma once
#include "Market.h"
#include "quote_t.h"
#include <string>

class Binance final : public Market
{
public:
    Binance(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
        : Market(name, id, fees, canShort, params)
    {
    }

    quote_t GetQuote(const std::string& currencyPair) override;

    double GetAvail(std::string currency) override;

    std::string SendLongOrder(std::string direction, double quantity, double price) override;

    std::string SendShortOrder(std::string direction, double quantity, double price) override;

    bool IsOrderComplete(std::string orderId) override;

    double GetActivePos() override;

    double GetLimitPrice(double volume, bool isBid) override;

    void testBinance();
};