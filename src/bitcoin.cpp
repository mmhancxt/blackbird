#include "bitcoin.h"
#include <cmath>

Bitcoin::Bitcoin(unsigned i, std::string n, const std::string& s, double f, bool h)
  //: currencyPair(ccyPair)
{
  id = i;
  exchName = n;
  symbol = s;
  fees = f;
  hasShort = h;
  bid = 0.0;
  ask = 0.0;
}

void Bitcoin::safeUpdateData(quote_t quote) {
  std::lock_guard<std::mutex> lock(m_feedMutex);
  bid = quote.bid();
  ask = quote.ask();
}

unsigned Bitcoin::getId() const { return id; }

double Bitcoin::safeGetBid()  const
{
  std::lock_guard<std::mutex> lock(m_feedMutex);
  return bid;
}

double Bitcoin::safeGetAsk()  const
{
  std::lock_guard<std::mutex> lock(m_feedMutex);
  return ask;
}

double Bitcoin::safeGetMidPrice() const
{
  std::lock_guard<std::mutex> lock(m_feedMutex);
  if (bid > 0.0 && ask > 0.0) {
    return (bid + ask) / 2.0;
  } else {
    return 0.0;
  }
}

std::string Bitcoin::getExchName()  const { return exchName; }

double Bitcoin::getFees()           const { return fees; }

bool Bitcoin::getHasShort()         const { return hasShort; }
