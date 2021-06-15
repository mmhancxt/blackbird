#include <string>
#include <fstream>

#pragma once

class Instrument
{
public:
    Instrument(const std::string& quote, const std::string& base) : m_quoteCurrency(quote), m_baseCurrency(base)
    {
    }

    std::string GetName() const { return ToString(); }
    std::string ToString() const
    {
        return std::string(m_quoteCurrency + m_baseCurrency);
    }

    const std::string& GetQuoteCurrency() const { return m_quoteCurrency; }
    const std::string& GetBaseCurrency() const { return m_baseCurrency; }

    friend std::ostream& operator<<(std::ostream& os, const Instrument& pair)
    {
        os << "[" << pair.GetQuoteCurrency() << "/" << pair.GetBaseCurrency() << "]";
        return os;
    }

private:
    std::string m_quoteCurrency;
    std::string m_baseCurrency;
};