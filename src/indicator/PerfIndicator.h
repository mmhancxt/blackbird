#pragma once

#include "indicator/IIndicator.h"
#include "ITimeTriggeredEvent.h"
#include <cmath>
#include <fstream>

class PerfIndicator : public IIndicator, public ITimeTriggeredEvent
{
public:
   PerfIndicator(const std::string& name, Instrument* instr, long long interval, long long duration);

   void Compute() override;

   void OnTimeTriggeredUpdate(const std::chrono::time_point<std::chrono::system_clock>& now) override;
private:
   std::chrono::milliseconds m_duration;
   double m_dampingFactor {0};
   double m_ewmaMid {0};
   std::ofstream m_file;
};
