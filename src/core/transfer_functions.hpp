#ifndef JABBER_TRANSFER_FUNCTIONS
#define JABBER_TRANSFER_FUNCTIONS

#include <span>

namespace jabber
{
/**
 * @defgroup tf_group Transfer Functions
 * @{
 * 
 * @details @todo \f$\chi=\frac{\frac{p'_{02}}{p_{02}}}{\frac{p'}{p_\infty}}\f$
 */

/**
 * @brief Apply the transfer function \f$\chi^*\f$, the analytical 
 * low-frequency limit form given in Equation 15 of Chaudhry and
 * Chandler, 2017.
 * 
 * @param mach_bar      Freestream Mach number.
 * @param gamma         Specific heat ratio.
 * @param speed         Wave speed, 'S' for slow, 'F' for fast.
 * 
 * @param powers        Powers to apply transfer function to.
 */
void LowFrequencyLimitTF(double mach_bar, double gamma, char speed,
                            std::span<double> powers);

/**
 * @brief Apply the transfer function \f$\chi\f$ obtained via 
 * re-dimensionalization of a fit of the curve in Figure 14b of 
 * Chaudhry and Chandler, 2017.
 * 
 * @param mach_bar      Freestream Mach number.
 * @param gamma         Specific heat ratio.
 * @param speed         Wave speed, 'S' for slow, 'F' for fast.
 * @param delta         Shock standoff distance from pitot probe face.
 * @param freqs         Frequencies, associated with each power in \p powers.
 * 
 * @param powers        Powers to apply transfer function to.
 */
void FlowNormalFitTF(double mach_bar, double gamma, char speed, 
                        double delta, std::span<const double> freqs,
                        std::span<double> powers);

/// @}
// end of tf_group

} // namespace jabber

#endif // JABBER_TRANSFER_FUNCTIONS
