#include <ctime>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>
#include "Feeder.h"
#include "time_fun.h"
#include "db_fun.h"
#include "parameters.h"

Feeder::Feeder(const Parameters &params, const std::vector<std::unique_ptr<Market>> &markets,
               std::ofstream &log)
    : m_params(params), m_markets(markets), m_logFile(log)
{
    // Connects to the SQLite3 database.
    // This database is used to collect bid and ask information
    // from the exchanges. Not really used for the moment, but
    // would be useful to collect historical bid/ask data.
    if (createDbConnection(m_params, m_dbConn) != 0)
    {
        m_logFile << "ERROR: cannot connect to the database \'" << m_params.dbFile
                  << "\'\n"
                  << std::endl;
        //exit(EXIT_FAILURE);
    }

    for (const auto &market : markets)
    {
        createTable(market->GetName(), m_params, m_dbConn);
    }
}

void Feeder::GetMarketData()
{
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
        m_logFile << "Running..." << std::endl;
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
            m_logFile << "WARNING: " << diffTime << " second(s) too late at " << printDateTime(currTime) << std::endl;
            timeinfo.tm_sec += (ceil(diffTime / m_params.interval) + 1) * m_params.interval;
            currTime = mktime(&timeinfo);
            sleep_for(secs(m_params.interval - (diffTime % m_params.interval)));
            m_logFile << std::endl;
        }
        else if (diffTime < 0)
        {
            sleep_for(secs(-diffTime));
        }
        // Header for every iteration of the loop
        if (m_params.verbose)
        {
            m_logFile << "[ " << printDateTime(currTime) << " ]" << std::endl;
        }
        // Gets the bid and ask of all the exchanges
        for (auto &market : m_markets)
        {
            const auto &marketName = market->GetName();

            const auto &dico = market->GetDico();
            if (market->SupportReuesetMultiSymbols())
            {
                std::vector<std::string> ccySymbols;
                if (m_params.tradedPair.empty())
                {
                    for (const auto& p : dico)
                    {
                        ccySymbols.push_back(p.first);
                    }
                }
                else
                {
                    for (const auto& ccyPair : m_params.tradedPair)
                    {
                        ccySymbols.push_back(ccyPair.GetName());
                    }
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
                for (const auto &currencyPair : m_params.tradedPair)
                {
                    const auto &ccyPair = currencyPair.ToString();
                    auto quote = market->GetQuote(ccyPair);
                    ProcessQuote(ccyPair, quote, marketName, dico, currTime);
                }
            }
        }
        if (m_params.verbose)
        {
            m_logFile << "   ----------------------------" << std::endl;
        }
    }
}

void Feeder::ProcessQuote(const std::string& ccyPair, const quote_t& quote,
    const std::string& marketName, const Dico& dico, time_t currTime)
{
    double bid = quote.bid();
    double ask = quote.ask();

    // Saves the bid/ask into the SQLite database
    addBidAskToDb(marketName, ccyPair,
                  printDateTimeDb(currTime), bid, ask, m_params, m_dbConn);

    // If there is an error with the bid or ask (i.e. value is null),
    // we show a warning but we don't stop the loop.
    if (bid == 0.0)
    {
        m_logFile << "   WARNING: " << marketName << " bid is null" << std::endl;
    }
    if (ask == 0.0)
    {
        m_logFile << "   WARNING: " << marketName << " ask is null" << std::endl;
    }
    // Shows the bid/ask information in the log file
    if (m_params.verbose)
    {
        m_logFile << "   " << marketName << ": \t"
                  << ccyPair << ": \t"
                  << std::setprecision(2)
                  << bid << " / " << ask << std::endl;
    }
    // Updates the Bitcoin vector with the latest bid/ask data
    auto it = dico.find(ccyPair);
    if (it != dico.end())
    {
        auto &ccyPairs = it->second;
        ccyPairs->safeUpdateData(quote);
        curl_easy_reset(m_params.curl);
    }
    else
    {
        m_logFile << "   ERROR: can't find ExchangePairs for " << ccyPair << " " << marketName << std::endl;
    }
}
