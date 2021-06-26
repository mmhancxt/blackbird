#include "Instrument.h"
#include <cmath>

void Instrument::SafeUpdateData(const Limit& limit) {
    std::lock_guard<std::mutex> lock(m_feedMutex);
    m_hasUpdate = true;
    m_limit = limit;
}

unsigned Instrument::GetId() const { return m_id; }

Limit Instrument::SafeGetBestLimit()
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    m_hasUpdate = false;
    return m_limit;
}

Limit Instrument::SafeGetBestLimitReadOnly()
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return m_limit;
}

double Instrument::SafeGetBid()  const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return 0;
}

double Instrument::SafeGetAsk()  const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return 0;
}

double Instrument::SafeGetMidPrice() const
{
    return 0;
    // std::lock_guard<std::mutex> lock(m_feedMutex);
    // if (m_bid > 0.0 && m_ask > 0.0) {
    //     return (m_bid + m_ask) / 2.0;
    // } else {
    //     return 0.0;
    // }
}

std::string Instrument::GetExchName()  const { return m_exchName; }

double Instrument::GetFees()           const { return m_fees; }

bool Instrument::GetHasShort()         const { return m_hasShort; }

bool Instrument::HasMarketUpdate() const
{
    std::lock_guard<std::mutex> lock(m_feedMutex);
    return m_hasUpdate;
}