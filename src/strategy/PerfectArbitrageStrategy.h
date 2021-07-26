#pragma once

struct Parameters;
class Market;
struct Result;

class PerfectArbitrageStrategy
{
public:
   PerfectArbitrageStrategy(Parameters& params, std::shared_ptr<spdlog::logger> log, 
         const std::unordered_map<std::string, std::unique_ptr<Market>>& markets, const std::set<std::string>& symbols);   
   
   void Poll();

private:
    Parameters& m_params;
    std::shared_ptr<spdlog::logger> m_log;
    const std::unordered_map<std::string, std::unique_ptr<Market>>& m_markets;
    const std::set<std::string>& m_symbols;
    std::unordered_map<std::string, Result> m_openOperations;
};
