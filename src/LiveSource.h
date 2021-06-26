#include "Market.h"
#include "parameters.h"
#include "unique_sqlite.hpp"
#include "ccapi_cpp/ccapi_session.h"
#include "spdlog/spdlog.h"

#pragma once

class Dico;
struct Limit;

class LiveSource : public ccapi::EventHandler
{
public:
    LiveSource(const Parameters& params, const std::unordered_map<std::string, std::unique_ptr<Market>>& markets,
        std::shared_ptr<spdlog::logger> log);

    ~LiveSource();

    void Subscribe();
    void GetMarketData();

    bool processEvent(const ccapi::Event& event, ccapi::Session* session) override;

private:
    void ProcessLimit(const std::string& ccyPair, const Limit& limit,
        const std::string& marketName, const Dico& dico, time_t currentTime);

private:
    const Parameters& m_params;
    const std::unordered_map<std::string, std::unique_ptr<Market>>& m_markets;
    std::shared_ptr<spdlog::logger> m_log;
    unique_sqlite m_dbConn;
    std::unique_ptr<ccapi::Session> m_session;
};
