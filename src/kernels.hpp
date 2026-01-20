#ifndef JABBER_KERNELS
#define JABBER_KERNELS

#include <cstddef>

namespace jabber
{

/**
   * @brief Generalized kernel function for evaluating perturbed base flow
   * using CPU vectorization.
   * 
   * 
   * @details This function was designed following Intel guidelines for 
   * vectorizing code - including that the innermost loop is the vectorized
   * one. As such, this function is templated with \p TDim for loop unrolling
   * on the momentum terms. Data is stored in a SoA-format for contiguous 
   * memory accesses across hardware threads.
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
   * @param wave_dirs        Acoustic wave normalized direction vectors
   *                         (k_hat), dimensioned as \p num_waves x \p TDim.
   * @param k_dot_x_p_phi    k·x+φ term computed for all waves at all points,
   *                         dimensioned as \p num_waves x \p num_pts but
   *                         flattened.
   * @param t                Time.
   * @param rho              Output flow density to compute, sized \p num_pts.
   * @param rhoV             Output flow momentum vector to compute,
   *                         dimensioned as \p TDim x \p num_pts but flattened.
   * @param rhoE             Output flow energy to compute, sized \p num_pts.
*/
template<int TDim>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                        const double p_bar, const double *U_bar, 
                        const double gamma, const int num_waves, 
                        const double *wave_amps, 
                        const double *wave_omegas,
                        const double *wave_dirs,
                        const double *__restrict__ k_dot_x_p_phi,
                        const double t,
                        double *__restrict__ rho,
                        double *__restrict__ rhoV,
                        double *__restrict__ rhoE);

} // namespace jabber

#endif // JABBER_KERNELS
