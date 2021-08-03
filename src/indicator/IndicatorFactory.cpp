#include "indicator/IndicatorFactory.h"
#include "indicator/IIndicator.h"
#include "indicator/PerfIndicator.h"
#include "Instrument.h"
#include "TimeTriggeredManager.h"

IIndicator* IndicatorFactory::CreateIndicator(const std::string& name, Instrument* instr)
{
   if (name == "PerfIndicator")
   {
      PerfIndicator* perfIndicator = new PerfIndicator(name, instr);
      IIndicator* indicator = perfIndicator;
      //ITimeTriggeredEvent* event = perfIndicator;
      //m_timeTriggeredManager.AddEvent(event);
      return indicator;
   }
   else
   {
      return nullptr;
   }
}
