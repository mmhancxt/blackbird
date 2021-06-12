#ifndef BITCOIN_H
#define BITCOIN_H

#include "quote_t.h"
#include "instrument.h"
#include <string>
#include <mutex>

// Contains all the information for a given exchange,
// like fees and wether we can short on that exchange.
// FIXME: short selling should depend on the currency.
// TODO: To rename to crypto
class Bitcoin {

  private:
    unsigned id;
    std::string exchName;
    Instrument currencyPair;
    double fees;
    bool hasShort;
    double bid, ask;
    mutable std::mutex m_feedMutex;

  public:
    Bitcoin(unsigned id, std::string n, const Instrument& ccyPair, double f, bool h);
    void safeUpdateData(quote_t quote);
    unsigned getId() const;
    double safeGetAsk() const;
    double safeGetBid() const;
    double safeGetMidPrice() const;
    std::string getExchName() const;
    double getFees() const;
    bool getHasShort() const;
};

#endif
