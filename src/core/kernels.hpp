#ifndef JABBER_KERNELS
#define JABBER_KERNELS

#include <cstddef>

namespace jabber
{

/**
   * @brief Kernel function for evaluating perturbed base flow.
   * 
   * @warning The  layout of \p k_dot_x_p_phi depends on \p TGridInnerLoop to
   * ensure contiguous memory accesses across hardware threads.
   * 
   * @details Compute the flowfield with acoustic forcing. This function was
   * designed for its inner loops to be auto-vectorized by the compiler. As 
   * such, it is templated with \p TDim for "loop unrolling" on the momentum
   * terms and, to support a variety of use cases, \p TGridInnerLoop to support
   * vectorization over either the grid points or over the waves on the series
   * summations.
   * 
   * Without an OpenMP build, it is adviseable to use the \p TGridInnerLoop =
   * true version when number of grid points per core is large relative to the 
   * number of waves, and use the \p TGridInnerLoop = false version when the
   * number of waves is large relative to the number of grid points per core.
   * 
   * With an OpenMP build, thread-parallelization is along the wave axis for 
   * \p TGridInnerLoop true, and along the grid point axis for
   * \p TGridInnerLoop false. In this case, one should test out different 
   * configurations for their specific case.
   *
   * 
   * @tparam TDim            Physical dimension.
   * @tparam TGridInnerLoop  If true, inner loop of series summation is over
   *                         the grid points. If false, is over the waves.
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
   *                         \f$\hat{k}\f$. For slow acoustic waves, this is
   *                         the **inverted** normalized wave direction vector,
   *                         \f$-\hat{k}\f$. This is dimensioned as \p TDim x 
   *                         \p num_waves but flattened.
   * @param k_dot_x_p_phi    \f$\vec{k}\cdot x+\phi\f$ term computed for all
   *                         waves at all points. **If \p TGridInnerLoop is true,
   *                         this must be dimensioned as a flattened
   *                         [ \p num_waves ][ \p num_pts ]. If
   *                          \p TGridInnerLoop is false, this must be 
   *                         dimensioned as a flattened
   *                         [ \p num_pts ][ \p num_waves ].
   * @param t                Time.
   * @param rho              Output flow density to compute, sized \p num_pts.
   * @param rhoV             Output flow momentum vector to compute,
   *                         dimensioned as \p TDim x \p num_pts but flattened.
   * @param rhoE             Output flow energy to compute, sized \p num_pts.
*/
template<std::size_t TDim, bool GridInnerLoop>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                        const double p_bar, const double *U_bar, 
                        const double gamma, const int num_waves, 
                        const double *__restrict__ wave_amps, 
                        const double *__restrict__ wave_omegas,
                        const double *__restrict__ mod_wave_dirs,
                        const double *__restrict__ k_dot_x_p_phi,
                        const double t,
                        double *__restrict__ rho,
                        double *__restrict__ rhoV,
                        double *__restrict__ rhoE);

} // namespace jabber

#endif // JABBER_KERNELS
