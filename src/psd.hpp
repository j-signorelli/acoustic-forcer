#ifndef JABBER_PSD
#define JABBER_PSD

#include <span>
#include <functional>

namespace jabber
{

/**
 * @defgroup psd_group Power Spectral Density (PSD) Discretization
 * 
 * @brief Tools to compute the energy-conserved powers of a discrete set of
 * frequencies from a PSD.
 * 
 * @details The power within generic frequency range \f$[f_1, f_2]\f$ of a 
 * continuous, one-sided PSD \f$S(f)\f$ is given by
 * 
 * \f[
 * P_{f_1\text{-}f_2}=\int_{f_1}^{f_2}S(f)df.
 * \f]
 * 
 * As outlined in Appendix B of Tam et al., 2010, "Continuation of the Near 
 * Acoustic Field of a Jet to the Far Field. Part I: Theory", a broadband
 * spectrum of acoustic waves can be formulated by discretizing the PSD
 * into a set of wave frequencies \f$f_k\f$ and ensuring energy conservation
 * by setting their amplitudes according to the power within a 
 * \f$\Delta f_k\f$. Note that for a PSD with units \f$V^2/\text{Hz}\f$, after
 * computing a discrete set of powers and applying any transfer function, 
 * the cosine wave amplitude can then be computed by
 * 
 * 
 * \f[
 * V_k=\sqrt{2P_k}.
 * \f]
 * 
 * To allow for an arbitrary discretization of frequencies, the PSD must
 * be represented in a continuous form, in which case this integral may then be
 * exactly evaluated for frequency bins. To support this, lightweight
 * classes are provided in @ref psd_exact_group to formulate a
 * continuous representation of a digitized or discrete PSD and then compute
 * "exact" powers. For general use cases, discretizations using quadrature on a 
 * provided continuous PSD function are given in @ref psd_quad_group.
 * 
 * @{
 * 
 * @defgroup psd_quad_group Quadrature-Based Discretization
 * @{
 *
 */


/**
 * @brief Compute energy-conserved powers from a PSD using a midpoint Riemann
 * sum.
 * 
 * @details Specifically, this function evaluates
 * 
 * \f[
 *    P_k=S(f_k)\Delta f_k,
 * \f]
 * 
 * where
 * 
 * \f[
 * \Delta f_k = 
 * \begin{cases}
 * (f_{k+1}-f_{k-1})/2 & 0<k<N, \\
 * (f_1-f_0)/2 & k=0, \\
 * (f_N-f_{N-1})/2 &k=N.
 * \end{cases}
 * \f] 
 * 
 * if \p log_scale is false. If \p log_scale is true, then the midpoints are
 * computed on a log10 scale.
 * 
 * @param freqs         Input discrete center frequencies in ascending order.
 * @param psd           Input PSD evaluated at \p freqs, \f$S(f_k)\f$.
 * @param powers        Output powers.
 * @param log_scale     True if midpoints taken on log10 scale, false
 *                      otherwise.
 */
void DiscretizePSDRiemann(std::span<const double> freqs,
                           std::span<const double> psd,
                           std::span<double> powers,
                           bool log_scale=false);

/**
 *  @} end of psd_quad_group
 * 
 * @defgroup psd_exact_group Continuous PSD Representations
 * @{
 */

/// Base abstract class for a PSD.
class BasePSD
{
public:
   /// Evaluate the PSD at \p x.
   virtual double operator() (double x) const = 0;
   
   /**
    * @brief Compute energy-conserved powers using exact integration.
    * 
    * @param freqs         Input discrete center frequencies in ascending
    *                      order.
    * @param powers        Output powers.
    */
   virtual void Discretize(std::span<const double> freqs, 
                              std::span<double> powers) const = 0;
};


/**
 * @brief Piecewise log-log interpolation of discrete PSD data.
 */
class PWLogLogPSD : public BasePSD
{
private:
   const std::vector<double> x_;
   const std::vector<double> y_;
public:
   PWLogLogPSD(std::span<const double> x, std::span<const double> y)
   : x_(x.begin(), x.end()), y_(y.begin(), y.end()) {}

   double operator() (double x) const override;

   void Discretize(std::span<const double> freqs, 
                              std::span<double> powers) const override;

};

/**
 *  @} end of psd_quad_group
 *  @} end of psd_group
 */

} // namespace jabber

#endif // JABBER_PSD
