#include "transfer_functions.hpp"

namespace jabber
{

void LowFrequencyLimitTF(double mach_bar, double gamma, char speed,
                           std::span<double> powers)
{
   const double sign = speed == 'S' ? -1 : 1;
   const double num = (mach_bar*mach_bar + sign*2.0*mach_bar - sign/mach_bar);
   const double denom = (gamma*mach_bar*mach_bar - (gamma - 1.0)/2.0);
   const double frac = num/denom;

   const double chi = frac*frac;
   for (double &power : powers)
   {
      power /= chi;
   }
}

void FlowNormalFitTF(double mach_bar, double gamma, char speed, 
                        double delta, std::span<const double> freqs,
                        std::span<double> powers)
{
   
}

} // namespace jabber
