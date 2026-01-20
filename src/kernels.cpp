#include "kernels.hpp"

#include <cmath>

namespace jabber
{

template<int TDim>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                             const double p_bar, const double *U_bar, 
                             const double gamma, const int num_waves, 
                             const double *wave_amps, const double *wave_omegas,
                             const double *wave_dirs,
                             const double *__restrict__ k_dot_x_p_phi,
                             const double t,
                             double *__restrict__ rho,
                             double *__restrict__ rhoV,
                             double *__restrict__ rhoE)
{
   // Initialize all output fields
   // THIS SHOULD BE VECTORIZED:
   for (std::size_t i = 0; i < num_pts; i++)
   {
      // Set \rho=\bar{\rho}
      rho[i] = rho_bar;

      // Unroll to ensure that loop over points (inner-most) is
      // vectorized, NOT over these dims.
      #pragma unroll
      for (std::size_t d = 0; d < TDim; d++)
      {
         // Set \rho\vec{u} = \vec{\bar{u}}
         rhoV[d*num_pts + i] = U_bar[d];
      }
      // Set \rho E = \bar{p}/(\gamma-1)
      rhoE[i] = p_bar/(gamma-1.0);
   }

   const double c_infty = std::sqrt(gamma*p_bar/rho_bar);

   // Add contribution of each wave
   for (int w = 0; w < num_waves; w++)
   {
      const double rho_fac = wave_amps[w]/(c_infty*c_infty);
      const double rhoV_fac = wave_amps[w]/(rho_bar*c_infty);
      const double rhoE_fac = wave_amps[w]/(gamma-1.0);

      const double omt = wave_omegas[w]*t;
      const std::size_t w_offset = w*num_pts;

      // THIS SHOULD BE VECTORIZED:
      for (std::size_t i = 0; i < num_pts; i++)
      {
         double cos_w = std::cos(k_dot_x_p_phi[w_offset + i] - omt);

         // Add: \rho += (1/c_\infty^2)p'_w*cos(...)
         rho[i] += rho_fac*cos_w;

         // Unroll to ensure that loop over points (inner-most) is
         // vectorized, NOT over these dims.
         #pragma unroll
         for (std::size_t d = 0; d < TDim; d++)
         {
            // Add: \rhoV += (1/(\bar{\rho}))
            rhoV[d*num_pts + i] += rhoV_fac*cos_w*wave_dirs[d*num_waves + w];
         }

         rhoE[i] += rhoE_fac*cos_w;

      }
   }

   // TODO: One more loop to postprocess (multiply rhoV by rho, do energy shit, etc.)

}

// Explicit instantiation for Dims 1-3
template void ComputeKernel<1>(const std::size_t, const double,
                                const double, const double *, 
                                const double, const int, 
                                const double *, 
                                const double *,
                                const double *,
                                const double *__restrict__,
                                const double,
                                double *__restrict__,
                                double *__restrict__,
                                double *__restrict__);
                                
template void ComputeKernel<2>(const std::size_t, const double,
                                const double, const double *, 
                                const double, const int, 
                                const double *, 
                                const double *,
                                const double *,
                                const double *__restrict__,
                                const double,
                                double *__restrict__,
                                double *__restrict__,
                                double *__restrict__);
template void ComputeKernel<3>(const std::size_t, const double,
                                const double, const double *, 
                                const double, const int, 
                                const double *, 
                                const double *,
                                const double *,
                                const double *__restrict__,
                                const double,
                                double *__restrict__,
                                double *__restrict__,
                                double *__restrict__);

} // namespace jabber
