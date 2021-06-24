#pragma once

#include "quote_t.h"
#include <string>
#include <mutex>

// Contains all the information for a given exchange,
// like fees and wether we can short on that exchange.
// FIXME: short selling should depend on the currency.
class Instrument
{
public:
    Instrument(unsigned id, std::string marketName, const std::string& symbol, const std::string& wsName, const std::string& base,
        const std::string& quote, double fees, bool hasShort)
        : m_id(id),
        m_exchName(marketName),
        m_symbol(symbol),
        m_wsName(wsName),
        m_baseCurrency(base),
        m_quoteCurrency(quote),
        m_fees(fees),
        m_hasShort(hasShort)
    {
    }

    void SafeUpdateData(quote_t quote);
    unsigned GetId() const;
    std::pair<double, double> SafeGetBidAsk();
    std::pair<double, double> SafeGetBidAskReadOnly();
    double SafeGetAsk() const;
    double SafeGetBid() const;
    double SafeGetMidPrice() const;
    std::string GetExchName() const;
    double GetFees() const;
    bool GetHasShort() const;

    bool HasMarketUpdate() const;

    std::string GetName() const { return ToString(); }
    std::string ToString() const
    {
        return std::string(m_quoteCurrency + m_baseCurrency);
    }

    const std::string& GetWSName() const { return m_wsName; }

    const std::string& GetQuoteCurrency() const { return m_quoteCurrency; }
    const std::string& GetBaseCurrency() const { return m_baseCurrency; }

    friend std::ostream& operator<<(std::ostream& os, const Instrument& pair)
    {
        os << "[" << pair.GetQuoteCurrency() << "/" << pair.GetBaseCurrency() << "]";
        return os;
    }

private:
    unsigned m_id {0};
    std::string m_exchName;
    std::string m_symbol;
    std::string m_wsName;
    double m_fees {0};
    bool m_hasShort {true};
    bool m_hasUpdate {false};
    double m_bid {0};
    double m_ask {0};
    std::string m_quoteCurrency;
    std::string m_baseCurrency;
    mutable std::mutex m_feedMutex;
};