#include "Wallet.h"
#include <ctime>
#include <fstream>
#include "time_fun.h"

bool Wallet::InitializeBalance(const std::string& asset, double free, double blocked)
{
   if (m_container.find(asset) != m_container.end())
   {
      m_log->error("Duplicat asset error {} for {}", asset, m_name);
      return false;
   }
   m_container[asset] = std::make_unique<Balance>(free, blocked);
   return true;
}

const Balance* Wallet::GetBalance(const std::string& asset) const
{
   const auto it = m_container.find(asset);
   if (it != m_container.end())
   {
      return it->second.get();
   }
   else
   {
      m_log->error("No balance found for asset {} in {}", asset, m_name);
      return nullptr;
   }
}

void Wallet::WriteBalanceToFile()
{
   std::string fileName = "output/balance_" + m_name + printDateTimeFileName() + ".csv"; 
   std::ofstream file;
   file.open(fileName);
   for (const auto& p : m_container)
   {
      auto& balance = p.second;
      file << p.first << "\t" << balance->Free << balance->Locked << "\n";
   }
   file.close();
}

