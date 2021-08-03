#pragma once
class IIndicator;
class Instrument;
class TimeTriggeredManager;

class IndicatorFactory
{
public:
   IndicatorFactory(TimeTriggeredManager& timeTriggeredManager) : m_timeTriggeredManager(timeTriggeredManager)
   {
   }

   IIndicator* CreateIndicator(const std::string& name, Instrument* instr);
private:
   TimeTriggeredManager& m_timeTriggeredManager;
};
