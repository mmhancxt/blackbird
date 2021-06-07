#pragma once
#include <fstream>
#include <vector>
#include <memory>

#include "Market.h"

struct Parameters;

class BlackBird
{
public:
    BlackBird(Parameters& params, std::ofstream& logStream, const std::string& logName)
        : m_params(params),
        m_log(logStream),
        m_logFileName(logName)
    {
    }

    bool Initialize();

    void Run();

    // This structure contains the balance of both exchanges,
    // *before* and *after* an arbitrage trade.
    // This is used to compute the performance of the trade,
    // by comparing the balance before and after the trade.
    struct Balance
    {
        double leg1, leg2;
        double leg1After, leg2After;
    };
private:
    void InitializeMarkets();

    void InitializeInstruments();

private:
    Parameters& m_params;
    std::ofstream& m_log;
    const std::string& m_logFileName;
    std::vector<std::unique_ptr<Market>> m_markets;
    //std::vector<ExchangePair> exchangePairs;

    bool m_inMarket { false };
};