#ifndef JABBER_KERNELS
#define JABBER_KERNELS

#include <cstddef>

namespace jabber
{

/**
   * @brief Generalized kernel function for evaluating perturbed base flow.
   * 
   * 
   * @details This function was designed following Intel guidelines for 
   * autovectorizable code.  Because only inner-most loops are candidates for
   * vectorization, this function is templated with \p TDim for "loop unrolling"
   * on the momentum terms. Vectorization is across \p num_pts as that is
   * expected to be the largest value, so data dimensioned by it must be
   * stored in an SoA-format for contiguous memory accesses across hardware
   * threads.
   * 
   * All inner loops have been verified to be vectorized by Intel `icpx` 
   * 2025.3.1 using the flags `-O3 -xhost`. Proper vectorization by Intel
   * compilers can be checked via:
   * 
   * ```
   * icpx -O3 -qopt-report=3 \
   *          -qopt-report-file="report.yaml" \
   *          -xhost \
   *          -c kernels.cpp
   * ```
   * 
   * 
   * @tparam TDim            Physical dimension.
   * 
   * @param num_pts          Number of physical points to evaluate at.
   * @param rho_bar          Base flow density.
   * @param p_bar            Base flow pressure.
   * @param U_bar            Base flow velocity.
   * @param gamma            Specific heat ratio.
   * @param num_waves        Number of acoustic waves to compute.
   * @param wave_amps        Acoustic wave amplitudes, sized \p num_waves.
   * @param wave_omegas      Acoustic wave angular frequencies, sized 
   *                         \p num_waves.
   * @param mod_wave_dirs    **Modified** acoustic wave normalized direction
   *                         vectors -- For fast acoustic waves, this is simply
   *                         just the normalized wave direction vector, 
   *                         $\hat{k}$. For slow acoustic waves, this is the
   *                         **inverted** normalized wave direction vector,
   *                         $-\hat{k}$. This is dimensioned as \p TDim x 
   *                         \p num_waves but flattened.
   * @param k_dot_x_p_phi    $\vec{k}\cdot x+\phi$ term computed for all waves
   *                         at all points, dimensioned as \p num_waves x 
   *                          \p num_pts but flattened.
   * @param t                Time.
   * @param rho              Output flow density to compute, sized \p num_pts.
   * @param rhoV             Output flow momentum vector to compute,
   *                         dimensioned as \p TDim x \p num_pts but flattened.
   * @param rhoE             Output flow energy to compute, sized \p num_pts.
*/
template<std::size_t TDim>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                        const double p_bar, const double *U_bar, 
                        const double gamma, const int num_waves, 
                        const double *wave_amps, 
                        const double *wave_omegas,
                        const double *mod_wave_dirs,
                        const double *__restrict__ k_dot_x_p_phi,
                        const double t,
                        double *__restrict__ rho,
                        double *__restrict__ rhoV,
                        double *__restrict__ rhoE);

} // namespace jabber

#endif // JABBER_KERNELS
