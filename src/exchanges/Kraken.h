#pragma once
#include "Market.h"
#include <string>

struct json_t;

class Kraken final : public Market
{
public:
    Kraken(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
        : Market(name, id, fees, canShort, params)
    {
    }

    bool RetrieveInstruments() override;

    bool InitializeWallet() override { return true; }

    // quote_t GetQuote(const std::string& currencyPair) override;

    // bool GetQuotesForMultiSymbols(const std::vector<std::string>& ccyPairs,
    //     std::unordered_map<std::string, quote_t>& quotes) override;

    double GetAvail(std::string currency) override;

    std::string SendLongOrder(std::string direction, double quantity, double price) override;

    std::string SendShortOrder(std::string direction, double quantity, double price) override;

    std::string SendOrder(std::string direction, double quantity, double price);

    bool IsOrderComplete(std::string orderId) override;

    double GetActivePos() override;

    double GetLimitPrice(double volume, bool isBid) override;

    json_t* authRequest(std::string request, std::string options = "");

    void testKraken();

private:
    std::unordered_map<std::string, std::string> m_altNameMap;
};
