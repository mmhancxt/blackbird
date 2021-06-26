#include "Coinbase.h"
#include "utils/restapi.h"
#include "unique_json.hpp"

static RestApi &queryHandle(const Parameters &params)
{
    static RestApi query("https://api.pro.coinbase.com",
                         params.cacert.c_str(), params.logger);
    return query;
}

bool Coinbase::RetrieveInstruments()
{
    auto &exchange = queryHandle(m_params);
    std::string x = "/products";
    unique_json root{exchange.getRequest(x)};

    size_t index;
    json_t *symbolInfo = nullptr;
    json_array_foreach(root.get(), index, symbolInfo)
    {
        std::string id = json_string_value(json_object_get(symbolInfo, "id"));
        const std::string status = json_string_value(json_object_get(symbolInfo, "status"));
        if (status != "online")
        {
            m_log->info("coinbase: {} trading phase is not TRADING[{}], skip", id, status);
            continue;
        }

        const std::string baseCcy = json_string_value(json_object_get(symbolInfo, "base_currency"));
        const std::string quoteCcy = json_string_value(json_object_get(symbolInfo, "quote_currency"));

        std::string symbol = baseCcy + quoteCcy;
        Instrument *instrument = new Instrument(m_id, m_name, symbol, id, baseCcy, quoteCcy, m_fees, m_canShort);
        m_dico.AddInstrument(symbol, instrument);
    }
    m_log->info("Coinbase dico complete");
    return true;

}