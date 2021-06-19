#include "Instrument.h"
#include <cmath>

void Instrument::SafeUpdateData(quote_t quote) {
    std::lock_guard<std::mutex> lock(m_feedMutex);
    m_hasUpdate = true;
    m_bid = quote.bid();
    m_ask = quote.ask();
}

unsigned Instrument::GetId() const { return m_id; }

std::pair<double, double> Instrument::SafeGetBidAsk()
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    m_hasUpdate = false;
    return {m_bid, m_ask};
}

double Instrument::SafeGetBid()  const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return m_bid;
}

double Instrument::SafeGetAsk()  const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return m_ask;
}

double Instrument::SafeGetMidPrice() const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    if (m_bid > 0.0 && m_ask > 0.0) {
        return (m_bid + m_ask) / 2.0;
    } else {
        return 0.0;
    }
}

std::string Instrument::GetExchName()  const { return m_exchName; }

double Instrument::GetFees()           const { return m_fees; }

bool Instrument::GetHasShort()         const { return m_hasShort; }

bool Instrument::HasMarketUpdate() const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return m_hasUpdate;
}