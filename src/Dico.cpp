#include "Dico.h"
#include "Instrument.h"

void Dico::AddInstrument(const std::string& symbol, Instrument* instr)
{
    m_container[symbol] = instr;
}

Instrument* Dico::GetInstrumentBySymbol(const std::string& symbol) const
{
    const auto it = m_container.find(symbol);
    if (it != m_container.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}