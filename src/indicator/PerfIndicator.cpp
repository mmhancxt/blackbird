#include "indicator/PerfIndicator.h"
#include "Instrument.h"
#include "time_fun.h"

PerfIndicator::PerfIndicator(const std::string& name, Instrument* instr, long long interval, long long duration)
   : IIndicator(name, instr),
   ITimeTriggeredEvent(std::chrono::milliseconds(interval)),
   m_duration(std::chrono::milliseconds(duration))
{
   m_dampingFactor = std::exp(-interval/(double)duration);
   std::string fileName = "data/" + instr->GetName() + ".csv";
   m_file.open(fileName);
   m_file << "time,bid,ask,mid,ewma" << std::endl;
}

void PerfIndicator::Compute()
{
   double mid = m_instr->SafeGetMidPrice();
   m_ewmaMid = m_dampingFactor * m_ewmaMid + (1 - m_dampingFactor) * mid;
   m_val = m_ewmaMid;
   m_quality = 1;
   std::cout << mid << "," << m_dampingFactor << "," << m_val << std::endl;
}

void PerfIndicator::OnTimeTriggeredUpdate(const std::chrono::time_point<std::chrono::system_clock>& now)
{
   Compute();
   std::time_t now_c = std::chrono::system_clock::to_time_t(now);
   m_file << printDateTimeCsv(now_c) << "," << m_instr->SafeGetBid() << "," << m_instr->SafeGetAsk() << "," << m_instr->SafeGetMidPrice() << "," << m_ewmaMid << std::endl;
}
