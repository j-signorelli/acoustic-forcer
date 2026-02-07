#ifndef JABBER_PSD
#define JABBER_PSD

#include "interpolant.hpp"

#include <span>
#include <map>
#include <cstdint>

namespace jabber
{

/**
 * @defgroup psd_group Power Spectral Density (PSD) Discretization
 * 
 * @brief Tools to compute the energy-conserved powers of a discrete set of
 * frequencies from a PSD.
 * 
 * @details 
 * ## Theory
 * The power within generic frequency range \f$[f_1, f_2]\f$ of a 
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
 * by setting their amplitudes according to the power within an interval 
 * \f$\Delta f_k\f$. Note that for a PSD with units \f$V^2/\text{Hz}\f$, after
 * computing a discrete set of powers and applying any transfer function, 
 * the cosine wave amplitude can then be computed by
 * 
 * 
 * \f[
 * V_k=\sqrt{2P_k}.
 * \f]
 * 
 * For an arbitrary discretization of frequencies in Jabber, the PSD must
 * be represented in a continuous form, in which case this integral may then be
 * exactly evaluated for frequency bins. To support this, lightweight
 * classes are provided in @ref psd_exact_group to formulate a
 * continuous representation of a digitized or discrete PSD and then compute
 * "exact" powers. For general use cases, discretizations using quadrature on a 
 * provided discrete PSD are given in @ref psd_quad_group.
 * 
 * ## Interval Method
 * @todo here
 */

/**
 * @brief Struct of Δf bin information for PSD discretization.
 * 
 * @ingroup psd_group
 */
struct Interval
{
public:
   /**
    * @brief Method for determining interval/bin Δf for a given center
    * frequency in discretization of a PSD.
    */
   enum class Method : std::uint8_t
   {
      /// Compute interval as midpoint between adjacent frequencies.
      Midpoint,

      /**
       * @brief Compute interval as midpoint **on a log10 scale** between
       * adjacent frequencies.
       */
      MidpointLog10,

      Size
   };

   /**
    * @brief Compute Interval Δf for given discrete frequency at index \p i
    * in \p freqs using \p method.
    * 
    * @details
    * For \ref Method::Midpoint :
    * \f[
    * \Delta f_k = 
    * \begin{cases}
    * (f_{k+1}-f_{k-1})/2 & 0<k<N, \\
    * (f_1+f_0)/2 & k=0, \\
    * (f_N-f_{N-1})/2 &k=N.
    * \end{cases}
    * \f]
    * 
    * For \ref Method::MidpointLog10 :
    * \f[
    * \Delta f_k =
    * \begin{cases}
    * \sqrt{f_kf_{k+1}}-\sqrt{f_kf_{k-1}} & 0<k<N, \\
    * \sqrt{f_0f_1}-f_0 & k=0, \\
    * f_N-\sqrt{f_Nf_{N-1}} &k=N.
    * \end{cases}
    * \f]
    * 
    * @param freqs        Span of all discrete frequencies, of size \f$N+1\f$.
    * @param i            Index of frequency to compute interval Δf for.
    * @param method       Method to use for interval computation.
    */
   static Interval ComputeInterval(std::span<const double> freqs,
                                    std::size_t i, Interval::Method method);


   /// Left-bound of interval.
   double f_left;

   /// Right-bound of interval.
   double f_right;

   /// Δf
   double df() const { return f_right - f_left; }
};

/** @{
 * 
 * @defgroup psd_quad_group Quadrature-Based Discretization
 * @{
 *
 */


/**
 * @brief Compute energy-conserved powers from a discrete PSD using a midpoint
 * Riemann sum.
 * 
 * @todo Not yet implemented!
 * 
 * @details Specifically, this function evaluates
 * 
 * \f[
 *    P_k=S(f_k)\Delta f_k,
 * \f]
 * where \f$\Delta f_k\f$ is determined by the \p method.
 * 
 * @param freqs         Input discrete center frequencies in ascending order.
 * @param psd           Input PSD evaluated at \p freqs, \f$S(f_k)\f$.
 * @param powers        Output powers.
 * @param method        Interval::Method enumerator.
 */
void DiscretizePSDRiemann(std::span<const double> freqs,
                           std::span<const double> psd,
                           std::span<double> powers,
                           Interval::Method method);

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

   /// Lower frequency bound of PSD.
   virtual double Min() const = 0;

   /// Upper frequency bound of PSD.
   virtual double Max() const = 0;
   
   /**
    * @brief Compute the power/energy from \p f1 to \p f2
    * 
    * @details Note that \p f1 and \p f2 should be within [ \ref Min() , 
    * \ref Max() ].
    */
   virtual double Integrate(double f1, double f2) const = 0;

   /**
    * @brief Compute energy-conserved powers using exact integration.
    * 
    * @details Integration is done from \ref Min() to \ref Max() on both sides.
    * 
    * @param freqs         Input discrete center frequencies in ascending
    *                      order, in range [ \ref Min(), \ref Max() ].
    * @param powers        Output powers.
    * @param method        Interval::Method enumerator.
    */
   void Discretize(std::span<const double> freqs, std::span<double> powers,
                                             Interval::Method method) const;
};

/**
 * @brief Piecewise linear interpolation of discrete PSD data.
 * 
 */
class PWLinearPSD : public PWLinear, public BasePSD
{

public:
   /**
    * @brief Construct a new PWLinearPSD object
    * 
    * @param freq     Set of discrete frequencies to fit lines in log space
    *                 to. PSD bounds \ref Min() and \ref Max() are defined by
    *                 the minimum and maximum discrete frequencies provided.
    * @param psd      PSD associated with each frequency in \p freq.
    */
   PWLinearPSD(std::span<const double> freq, std::span<const double> psd)
   : PWLinear(freq, psd) {}

   double Min() const override 
   { 
      return Map().begin()->first;
   }

   double Max() const override 
   { 
      return std::prev(Map().end())->first;
   }

   double Integrate(double f1, double f2) const override;
};

/**
 * @brief Piecewise log-log interpolation of discrete PSD data.
 */
class PWLogLogPSD : public PWLogLog, public BasePSD
{
public:

   /**
    * @brief Construct a new PWLogLogPSD object
    * 
    * @param freq     Set of discrete frequencies to fit lines in log space
    *                 to. PSD bounds \ref Min() and \ref Max() are defined by
    *                 the minimum and maximum discrete frequencies provided.
    * @param psd      PSD associated with each frequency in \p freq.
    */
   PWLogLogPSD(std::span<const double> freq, std::span<const double> psd)
   : PWLogLog(freq, psd) {}

   double Min() const override 
   { 
      return Map().begin()->first;
   }

   double Max() const override 
   { 
      return std::prev(Map().end())->first;
   }

   double Integrate(double f1, double f2) const override;
   
};

/**
 *  @} end of psd_quad_group
 *  @} end of psd_group
 */

} // namespace jabber

#endif // JABBER_PSD
