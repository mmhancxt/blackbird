#pragma once
#include <vector>
#include <memory>
#include <set>
#include <unordered_map>
#include "spdlog/spdlog.h"
#include "Market.h"
#include "TimeTriggeredManager.h"

struct Parameters;
class Dico;
class PerfectArbitrageStrategy;

class BlackBird
{
public:
    BlackBird(Parameters& params, std::shared_ptr<spdlog::logger> logger, const std::string& logName)
        : m_params(params),
        m_log(logger),
        m_logFileName(logName)
    {
    }

    bool Initialize();

    void Run();

    // This structure contains the balance of both exchanges,
    // *before* and *after* an arbitrage trade.
    // This is used to compute the performance of the trade,
    // by comparing the balance before and after the trade.
    // struct Balance
    // {
    //     double leg1, leg2;
    //     double leg1After, leg2After;
    // };
private:
    bool InitializeMarkets();

    void InitializeInstruments();

    bool InitializeIndicators();

    bool InitializeStrategies();

private:
    Parameters& m_params;
    std::shared_ptr<spdlog::logger> m_log;
    const std::string& m_logFileName;
    std::unordered_map<std::string, std::unique_ptr<Market>> m_markets;
    std::set<std::string> m_allSubscriptionSymbols;
    TimeTriggeredManager m_timeTriggeredManager;
    PerfectArbitrageStrategy* m_strategy { nullptr };

    bool m_inMarket { false };
};
