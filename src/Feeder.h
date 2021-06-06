#include "market.h"
#include "parameters.h"
#include "unique_sqlite.hpp"

#pragma once

class Feeder
{
public:
    Feeder(const Parameters& params, std::ofstream& log);

    void GetMarketData(const std::vector<ExchangePair>& exchangePairs);

    unique_sqlite& GetSqliteDBConn() { return m_dbConn; }

    getQuoteType getQuote[2];
    std::string dbTableName[2];
private:
    const Parameters& m_params;
    std::ofstream& m_logFile;
    unique_sqlite m_dbConn;
};