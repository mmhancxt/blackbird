#pragma once
#include <map>

class ITimeTriggeredEvent;

class TimeTriggeredManager
{
public:
   TimeTriggeredManager() = default;
   void AddEvent(ITimeTriggeredEvent* event);

   void Work();

private:
   std::map<ITimeTriggeredEvent*, std::chrono::time_point<std::chrono::system_clock>> m_events;
};
