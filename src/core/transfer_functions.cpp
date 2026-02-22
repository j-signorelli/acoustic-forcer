#include "transfer_functions.hpp"

namespace jabber
{

double LowFrequencyLimitTF(double mach_bar, double gamma, char speed)
{
   const double sign = speed == 'S' ? -1 : 1;
   const double num = (mach_bar*mach_bar + sign*2.0*mach_bar - sign/mach_bar);
   const double denom = (gamma*mach_bar*mach_bar - (gamma - 1.0)/2.0);
   const double frac = num/denom;
   return frac*frac;
}

} // namespace jabber
