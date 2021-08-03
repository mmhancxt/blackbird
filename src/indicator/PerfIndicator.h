#pragma once

#include "indicator/IIndicator.h"

class PerfIndicator : public IIndicator
{
public:
   PerfIndicator(const std::string& name, Instrument* instr) : IIndicator(name, instr)
   {
   }

   void Compute() override;
};
