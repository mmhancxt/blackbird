#pragma once

class Instrument;
class IIndicator
{
public:
   IIndicator(const std::string& name, Instrument* instr) : m_name(name), m_instr(instr)
   {
   }

   virtual ~IIndicator() {}

   virtual double GetValue() { return m_val; }
   virtual double GetQuality() { return m_quality; }

   virtual void Compute() = 0;

protected:
   std::string m_name;
   Instrument* m_instr;
   double m_val = 0;
   double m_quality = 0;
};
