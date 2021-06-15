#include <string>
#include "parameters.h"
#include "quote_t.h"
#include "bitcoin.h"
#include <unordered_map>
#include <set>
#pragma once

using Dico = std::unordered_map<std::string, Bitcoin*>;

class Market
{
public:
    Market(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
        : m_name(name), m_id(id), m_fees(fees), m_canShort(canShort), m_params(params), m_log(*params.logFile)
    {
    }

    virtual ~Market() = default;

    const std::string& GetName() const { return m_name; }
    int GetID() const { return m_id; }
    double GetFees() const { return m_fees; }
    bool CanShort() const { return m_canShort; }
    void SetRequestMultiSymbols(bool val) { m_requestMultiSymbols = val; }
    bool SupportReuesetMultiSymbols() const { return m_requestMultiSymbols; }

    const std::set<std::string>& GetRawSymbols() const { return m_rawSymbols; }
    Dico& GetDico() { return m_dico; }

    virtual bool RetrieveInstruments() = 0;

    void InitializeInstruments(const std::set<std::string> &symbols)
    {
        if (m_params.tradedPair.empty())
        {
            for (const auto &symbol : symbols)
            {
                Bitcoin *cryptoCcy = new Bitcoin(m_id, m_name, symbol, m_fees, m_canShort);
                m_dico[symbol] = cryptoCcy;
            }
        }
        else
        {
            for (const auto &pair : m_params.tradedPair)
            {
                const auto symbol = pair.GetName();
                Bitcoin *cryptoCcy = new Bitcoin(m_id, m_name, symbol, m_fees, m_canShort);
                m_dico[symbol] = cryptoCcy;
            }
        }
    }

    virtual quote_t GetQuote (const std::string& currencyPair) = 0;
    virtual bool GetQuotesForMultiSymbols(const std::vector<std::string>& ccyPairs,
        std::unordered_map<std::string, quote_t>& quotes)
    {
        return false;
    }
    virtual double GetAvail(std::string currency) = 0;
    virtual std::string SendLongOrder(std::string direction, double quantity, double price) = 0;
    virtual std::string SendShortOrder(std::string direction, double quantity, double price) = 0;
    virtual bool IsOrderComplete(std::string orderId) = 0;
    virtual double GetActivePos() = 0;
    virtual double GetLimitPrice(double volume, bool isBid) = 0;

protected:

    std::string m_name;
    int m_id { 0 };
    const Parameters& m_params;
    std::ofstream& m_log;
    bool m_canShort { true };
    double m_fees { 0 };
    bool m_requestMultiSymbols { false };
    std::set<std::string> m_rawSymbols;
    Dico m_dico;
};
