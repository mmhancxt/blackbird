#ifndef DB_FUN_H
#define DB_FUN_H

#include <string>
#include "unique_sqlite.hpp"

struct Parameters;

int createDbConnection(const Parameters& params, unique_sqlite& dbConn);

int createTable(std::string exchangeName, const Parameters& params, unique_sqlite& dbConn);

int addBidAskToDb(const std::string& exchangeName, const std::string& currencyPair,
    std::string datetime, double bidPrice, double bidQty, double askPrice, double askQty,
    const Parameters& params, unique_sqlite& dbConn);

#endif
