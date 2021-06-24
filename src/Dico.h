#pragma once

#include <unordered_map>

class Instrument;

class Dico
{
public:
    Dico() = default;
    ~Dico() = default;

    using container_type = std::unordered_map<std::string, Instrument*>;

    const container_type& GetAllInstruments() const { return m_container; }

    void AddInstrument(const std::string& symbol, Instrument* instr);
    Instrument* GetInstrumentBySymbol(const std::string& symbol) const;

private:
    container_type m_container;
};