#include "Market.h"
#include "parameters.h"
#include "unique_sqlite.hpp"

#pragma once

class Feeder
{
public:
    Feeder(const Parameters& params, const std::vector<std::unique_ptr<Market>>& markets,
        std::ofstream& log);

    void GetMarketData();

private:
    void ProcessQuote(const std::string& ccyPair, const quote_t& quote,
        const std::string& marketName, const Dico& dico, time_t currentTime);

private:
    const Parameters& m_params;
    const std::vector<std::unique_ptr<Market>>& m_markets;
    std::ofstream& m_logFile;
    unique_sqlite m_dbConn;
};
