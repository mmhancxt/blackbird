#pragma once

#include <chrono>

class ITimeTriggeredEvent
{
public:
   ITimeTriggeredEvent(std::chrono::milliseconds interval) : m_inverval(interval)
   {
   }

   virtual ~ITimeTriggeredEvent() = default;

   virtual void OnTimeTriggeredUpdate(const std::chrono::time_point<std::chrono::system_clock>& now) = 0;

   const std::chrono::milliseconds GetInterval() const { return m_inverval; }

protected:
   std::chrono::milliseconds m_inverval;
};
