#pragma once
#include <unordered_map>
#include <memory>

struct Balance
{
   Balance(double _free, double _locked) : Free(_free), Locked(_locked) {}

   double Free;
   double Locked;
};

class Wallet
{
public:
   using WalletContainer = std::unordered_map<std::string, std::unique_ptr<Balance>>;

   Wallet(const std::string& name, std::shared_ptr<spdlog::logger> log) : m_name(name), m_log(log)
   {
   }

   bool InitializeBalance(const std::string& asset, double free, double blocked);

   const Balance* GetBalance(const std::string& asset) const;

   void WriteBalanceToFile();

private:
   std::string m_name;
   WalletContainer m_container;
   std::shared_ptr<spdlog::logger> m_log;
};
