#pragma once 

class Instrument;
class IIndicator
{
public:
   IIndicator(const std::string& name, Instrument* instr) : m_name(name), m_instr(instr)
   {
   }

   virtual ~IIndicator() {}

   virtual double GetValue() { return val; }
   virtual double GetQuality() { return quality; }

   virtual void Compute() = 0;

protected:
   std::string m_name;      
   Instrument* m_instr;
   double val = 0;
   double quality = 0;
};
