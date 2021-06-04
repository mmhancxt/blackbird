#ifndef BITCOIN_H
#define BITCOIN_H

#include "quote_t.h"
#include "instrument.h"
#include <string>

// Contains all the information for a given exchange,
// like fees and wether we can short on that exchange.
// FIXME: short selling should depend on the currency.
// TODO: To rename to crypto
class Bitcoin {

  private:
    unsigned id;
    std::string exchName;
    CurrencyPair currencyPair;
    double fees;
    bool hasShort;
    bool isImplemented;
    double bid, ask;

  public:
    Bitcoin(unsigned id, std::string n, const CurrencyPair& ccyPair, double f, bool h, bool m);
    void updateData(quote_t quote);
    unsigned getId() const;
    double getAsk() const;
    double getBid() const;
    double getMidPrice() const;
    std::string getExchName() const;
    double getFees() const;
    bool getHasShort() const;
    bool getIsImplemented() const;
};

#endif
