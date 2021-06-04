#include <string>
#include <fstream>

#pragma once

class CurrencyPair
{
public:
    CurrencyPair(const std::string& quote, const std::string& base) : m_quoteCurrency(quote), m_baseCurrency(base)
    {
    }

    std::string ToString() const
    {
        return std::string(m_quoteCurrency + m_baseCurrency);
    }

    const std::string& GetQuoteCurrency() const { return m_quoteCurrency; }
    const std::string& GetBaseCurrency() const { return m_baseCurrency; }

    friend std::ostream& operator<<(std::ostream& os, const CurrencyPair& pair)
    {
        os << "[" << pair.GetQuoteCurrency() << "/" << pair.GetBaseCurrency() << "]";
        return os;
    }

private:
    std::string m_quoteCurrency;
    std::string m_baseCurrency;
};