#ifndef JABBER_TRANSFER_FUNCTIONS
#define JABBER_TRANSFER_FUNCTIONS

namespace jabber
{

/**
 * @brief Compute the transfer function $\chi$ using the analytical 
 * low-frequency limit form in Chaudhry and Chandler, 2017.
 * 
 * @param mach_bar      Freestream Mach number.
 * @param gamma         Specific heat ratio.
 * @param speed         Wave speed, 'S' for slow, 'F' for fast.
 * @return double       $\chi$.
 */
double LowFrequencyLimitTF(double mach_bar, double gamma, char speed);

} // namespace jabber

#endif // JABBER_TRANSFER_FUNCTIONS
