#pragma once
#include "Market.h"
#include <string>

class Coinbase final : public Market
{
public:
    Coinbase(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
        : Market(name, id, fees, canShort, params)
    {
    }

    bool RetrieveInstruments() override;

    double GetAvail(std::string currency) override { return 0; };

    std::string SendLongOrder(std::string direction, double quantity, double price) override { return "";}

    std::string SendShortOrder(std::string direction, double quantity, double price) override { return "";}

    bool IsOrderComplete(std::string orderId) override { return true; }

    double GetActivePos() override { return 0; }

    double GetLimitPrice(double volume, bool isBid) override { return 0; }
};