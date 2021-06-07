#include <string>
#include "parameters.h"
#include "quote_t.h"
#include "bitcoin.h"
#include <unordered_map>
#pragma once

using Dico = std::unordered_map<std::string, Bitcoin*>;

class Market
{
public:
    Market(const std::string& name, int id, double fees, bool canShort, const Parameters& params)
        : m_name(name), m_id(id), m_fees(fees), m_canShort(canShort), m_params(params)
    {
    }

    virtual ~Market() = default;

    const std::string& GetName() const { return m_name; }
    int GetID() const { return m_id; }
    double GetFees() const { return m_fees; }
    bool CanShort() const { return m_canShort; }
    void SetRequestMultiSymbols(bool val) { m_requestMultiSymbols = val; }
    bool SupportReuesetMultiSymbols() const { return m_requestMultiSymbols; }
    Dico& GetDico() { return m_dico; }

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

    Dico m_dico;
    std::string m_name;
    int m_id { 0 };
    const Parameters& m_params;
    bool m_canShort { true };
    double m_fees { 0 };
    bool m_requestMultiSymbols { false };
};
