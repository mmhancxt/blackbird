#include "parameters.h"
#include "unique_sqlite.hpp"

#include <iostream>
#include <string>


// Defines some helper overloads to ease sqlite resource management
namespace {
template <typename UNIQUE_T>
class sqlite_proxy {
  typename UNIQUE_T::pointer S;
  UNIQUE_T &owner;

public:
  sqlite_proxy(UNIQUE_T &owner) : S(nullptr), owner(owner)
  {}
  ~sqlite_proxy()                           { owner.reset(S); }
  operator typename UNIQUE_T::pointer* ()   { return &S;      }
};

template <typename T, typename deleter>
sqlite_proxy< std::unique_ptr<T, deleter> >
acquire(std::unique_ptr<T, deleter> &owner) { return owner;   }
}

int createDbConnection(const Parameters& params, unique_sqlite& dbConn) {
  int res = sqlite3_open(params.dbFile.c_str(), acquire(dbConn));

  if (res != SQLITE_OK)
    std::cerr << sqlite3_errmsg(dbConn.get()) << std::endl;

  return res;
}

int createTable(std::string exchangeName, const Parameters& params, unique_sqlite& dbConn) {

  std::string query = "CREATE TABLE IF NOT EXISTS `" + exchangeName +
                      "` (CurrencyPair TEXT NOT NULL, Datetime DATETIME NOT NULL, bid DECIMAL(8, 2), ask DECIMAL(8, 2));";
  unique_sqlerr errmsg;
  int res = sqlite3_exec(dbConn.get(), query.c_str(), nullptr, nullptr, acquire(errmsg));
  if (res != SQLITE_OK)
    std::cerr << errmsg.get() << std::endl;

  return res;
}

int addBidAskToDb(const std::string& exchangeName, const std::string& currencyPair,
  std::string datetime, double bid, double ask, const Parameters& params, unique_sqlite& dbConn)
{
  std::string query = "INSERT INTO `" + exchangeName +
                      "` VALUES ('"   + currencyPair + "','" + datetime +
                      "'," + std::to_string(bid) +
                      "," + std::to_string(ask) + ");";
  unique_sqlerr errmsg;
  int res = sqlite3_exec(dbConn.get(), query.c_str(), nullptr, nullptr, acquire(errmsg));
  if (res != SQLITE_OK)
    std::cerr << errmsg.get() << std::endl;

  return res;
}
