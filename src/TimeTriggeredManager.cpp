#include "TimeTriggeredManager.h"
#include "ITimeTriggeredEvent.h"

void TimeTriggeredManager::AddEvent(ITimeTriggeredEvent* event)
{
   const std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
   m_events[event] = now;
   event->TimeTriggered(now);
}

void TimeTriggeredManager::Work()
{
   const std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();
   for (auto it = m_events.begin(); it != m_events.end(); ++it)
   {
      auto* event = it->first;
      const auto& lastTriggeredTime = it->second;
      if (now - lastTriggeredTime >= event->GetInterval())
      {
         m_events[event] = now;
         event->TimeTriggered(now);
      }
   }
}
