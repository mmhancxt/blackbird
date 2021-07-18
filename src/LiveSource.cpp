#include <ctime>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
#include "LiveSource.h"
#include "time_fun.h"
#include "db_fun.h"
#include "parameters.h"
#include "Limit.h"
#include "ccapi_cpp/ccapi_session.h"

LiveSource::LiveSource(const Parameters &params, const std::unordered_map<std::string, std::unique_ptr<Market>> &markets,
               std::shared_ptr<spdlog::logger> logger)
    : m_params(params), m_markets(markets), m_log(logger)
{
    // Connects to the SQLite3 database.
    // This database is used to collect bid and ask information
    // from the exchanges. Not really used for the moment, but
    // would be useful to collect historical bid/ask data.
    if (createDbConnection(m_params, m_dbConn) != 0)
    {
        m_log->error("cannot connect to the database {}", m_params.dbFile);
        //exit(EXIT_FAILURE);
    }

    for (const auto &p: markets)
    {
        createTable(p.first, m_params, m_dbConn);
    }
}

LiveSource::~LiveSource()
{
    m_session->stop();
}


void LiveSource::Subscribe()
{
    using namespace ccapi;

    m_log->info("Start to subscrib market data");
    std::map<std::string, std::map<std::string, std::string>> exchangeInstrumentSymbolMap;
    std::vector<Subscription> subscriptionList;
    for (const auto &p: m_markets)
    {
        const auto& marketName = p.first;
        const auto& market = p.second;
        const auto& dico = market->GetDico();
        for (const auto &symbol : market->GetSubscriptionSymbols())
        {
            const auto& instr = dico.GetInstrumentBySymbol(symbol);
            if (instr == nullptr)
            {
                m_log->error("failed to find {} in dico of {}", symbol,  marketName);
                continue;
            }
            const std::string subscriptionID = marketName + "|" + symbol;
            std::string option = "";
            if (m_params.mdUpdateIntervalMs != 0)
            {
               option += "CONFLATE_INTERVAL_MILLISECONDS=";
               option += std::to_string(m_params.mdUpdateIntervalMs);
            }
            Subscription subscription(marketName, symbol, "MARKET_DEPTH", option, subscriptionID);
            exchangeInstrumentSymbolMap[marketName][symbol] = instr->GetWSName();
            subscriptionList.push_back(subscription);
        }
    }

    SessionOptions sessionOptions;
    SessionConfigs sessionConfigs(exchangeInstrumentSymbolMap);
    m_session.reset(new Session(sessionOptions, sessionConfigs, this));
    m_session->subscribe(subscriptionList);
}

bool LiveSource::processEvent(const ccapi::Event& event, ccapi::Session* session)
{
    using namespace ccapi;

    if (event.getType() == Event::Type::SUBSCRIPTION_DATA)
    {
        for (const auto &message : event.getMessageList())
        {
            const auto& correlationId = message.getCorrelationIdList().at(0);
            auto pos = correlationId.find('|');
            if (pos != std::string::npos)
            {
                const auto marketName = correlationId.substr(0, pos);
                const auto symbol = correlationId.substr(pos+1);
                //m_log << "DEBUG : " << marketName << " : " << symbol << std::endl;
                const auto it = m_markets.find(marketName);
                Market* market = nullptr;
                if (it != m_markets.end())
                {
                    market = it->second.get();
                }
                else
                {
                    m_log->error("Market name not found {}", marketName);
                    continue;
                }
                const auto& dico = market->GetDico();
                Instrument* instr = dico.GetInstrumentBySymbol(symbol);
                if (instr == nullptr)
                {
                    m_log->error("can't find instrument {}", symbol);
                    continue;
                }
                //m_log->info("{}: Best bid and ask at {} are:", marketName, UtilTime::getISOTimestamp(message.getTime()));
                double bidPrice = 0, bidSize = 0, askPrice = 0, askSize = 0;
                for (const auto &element : message.getElementList())
                {
                    const std::map<std::string, std::string> &elementNameValueMap = element.getNameValueMap();
                    //m_log->info("  {}", toString(elementNameValueMap));

                    if (element.has("BID_PRICE"))
                    {
                        bidPrice = stod(element.getValue("BID_PRICE"));
                    }
                    if (element.has("ASK_PRICE"))
                    {
                        askPrice = stod(element.getValue("ASK_PRICE"));
                    }
                    if (element.has("BID_SIZE"))
                    {
                        bidSize = stod(element.getValue("BID_SIZE"));
                    }
                    if (element.has("ASK_SIZE"))
                    {
                        askSize = stod(element.getValue("ASK_SIZE"));
                    }
                }
                Limit limit(bidPrice, bidSize, askPrice, askSize);
                ProcessLimit(symbol, limit, marketName, dico, std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::microseconds>(message.getTime())));
            }
            else
            {
                m_log->error("invalid correlationId {}", correlationId);
            }
        }
    }
    return true;
}


void LiveSource::GetMarketData()
{
    /*
    time_t rawtime = time(nullptr);
    tm timeinfo = *localtime(&rawtime);
    using std::this_thread::sleep_for;
    using millisecs = std::chrono::milliseconds;
    using secs = std::chrono::seconds;
    // Waits for the next 'interval' seconds before starting the loop
    while ((int)timeinfo.tm_sec % m_params.interval != 0)
    {
        sleep_for(millisecs(100));
        time(&rawtime);
        timeinfo = *localtime(&rawtime);
    }
    if (!m_params.verbose)
    {
        m_log << "Running..." << std::endl;
    }

    bool stillRunning = true;
    time_t currTime;
    time_t diffTime;

    while (true)
    {
        currTime = mktime(&timeinfo);
        time(&rawtime);
        diffTime = difftime(rawtime, currTime);
        // Checks if we are already too late in the current iteration
        // If that's the case we wait until the next iteration
        // and we show a warning in the log file.
        if (diffTime > 0)
        {
            m_log << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
            timeinfo.tm_sec += (ceil(diffTime / m_params.interval) + 1) * m_params.interval;
            currTime = mktime(&timeinfo);
            sleep_for(secs(m_params.interval - (diffTime % m_params.interval)));
            m_log << std::endl;
        }
        else if (diffTime < 0)
        {
            sleep_for(secs(-diffTime));
        }
        // Header for every iteration of the loop
        if (m_params.verbose)
        {
            m_log << "[ " << printDateTime(currTime) << " ]" << std::endl;
        }
        // Gets the bid and ask of all the exchanges
        for (auto &p : m_markets)
        {
            const auto &marketName = p.first;
            auto& market = p.second;

            const auto &dico = market->GetDico();
            if (market->SupportReuesetMultiSymbols())
            {
                std::vector<std::string> ccySymbols;
                if (m_params.tradedPair.empty())
                {
                    for (const auto& p : dico.GetAllInstruments())
                    {
                        ccySymbols.push_back(p.first);
                    }
                }
                else
                {
                    ccySymbols = m_params.tradedPair;
                }
                std::unordered_map<std::string, quote_t> quotes;

                market->GetQuotesForMultiSymbols(ccySymbols, quotes);
                for (const auto & kv : quotes)
                {
                    ProcessQuote(kv.first, kv.second, marketName, dico, currTime);
                }
            }
            else
            {
                for (const auto &ccyPair : m_params.tradedPair)
                {
                    auto quote = market->GetQuote(ccyPair);
                    ProcessQuote(ccyPair, quote, marketName, dico, currTime);
                }
            }
        }
        if (m_params.verbose)
        {
            m_log << "   ----------------------------" << std::endl;
        }
    }*/
}

void LiveSource::ProcessLimit(const std::string& ccyPair, const Limit& limit,
    const std::string& marketName, const Dico& dico, time_t currTime)
{
    const auto& bid = limit.Bid;
    const auto& ask = limit.Ask;
    const auto bidPrice = bid.Price;
    const auto bidQty = bid.Quantity;
    const auto askPrice = ask.Price;
    const auto askQty = ask.Quantity;

    // Saves the bid/ask into the SQLite database
    addBidAskToDb(marketName, ccyPair,
                  printDateTimeDb(currTime), bidPrice, bidQty, askPrice, askQty, m_params, m_dbConn);

    // If there is an error with the bid or ask (i.e. value is null),
    // we show a warning but we don't stop the loop.
    if (bidPrice == 0.0)
    {
        m_log->warn("{} bid is null", marketName);
    }
    if (askPrice == 0.0)
    {
        m_log->warn("{} ask is null", marketName);
    }
    // Shows the bid/ask information in the log file
    if (m_params.verbose)
    {
        std::stringstream ss;
        ss << "   " << marketName << ": \t"
                  << ccyPair << ": \t"
                  << std::setprecision(8)
                  << bidPrice << " / " << askPrice << std::endl;
        m_log->info(ss.str());
    }
    // Updates the Instrument vector with the latest bid/ask data
    auto* instr = dico.GetInstrumentBySymbol(ccyPair);
    if (instr != nullptr)
    {
        instr->SafeUpdateData(limit);
        curl_easy_reset(m_params.curl);
    }
    else
    {
        m_log->error("can't find ExchangePairs for {} {}", ccyPair, marketName);
    }
}
