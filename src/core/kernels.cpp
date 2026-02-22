#include "kernels.hpp"

#include <cmath>

namespace jabber
{

template<std::size_t TDim>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                     const double p_bar, const double *U_bar, 
                     const double gamma, const int num_waves, 
                     const double *wave_amps, const double *wave_omegas,
                     const double *mod_wave_dirs,
                     const double *__restrict__ k_dot_x_p_phi,
                     const double t,
                     double *__restrict__ rho,
                     double *__restrict__ rhoV,
                     double *__restrict__ rhoE)
{
   const double rhoE_init = p_bar/(gamma-1.0);
   
   for (std::size_t i = 0; i < num_pts; i++)
   {
      rho[i] = rho_bar;

      rhoV[i] = U_bar[0];
      if constexpr(TDim > 1)
      {
         rhoV[num_pts + i] = U_bar[1];
      }
      if constexpr(TDim > 2)
      {
         rhoV[2*num_pts + i] = U_bar[2];
      }
      rhoE[i] = rhoE_init;
   }

   const double c_infty = std::sqrt(gamma*p_bar/rho_bar);
   const double c_infty_sq = c_infty*c_infty;
   const double rho_bar_t_c_infty = rho_bar*c_infty;
   const double gamma_m_1 = gamma-1.0;

   // Add contribution of each wave
#ifdef JABBER_WITH_OPENMP
   #pragma omp parallel for reduction(+:rho[0:num_pts],\
                                        rhoV[0:num_pts*TDim],\
                                        rhoE[0:num_pts])
#endif // JABBER_WITH_OPENMP
   for (int w = 0; w < num_waves; w++)
   {
      const double rho_fac = wave_amps[w]/c_infty_sq;
      const double rhoV_fac = wave_amps[w]/rho_bar_t_c_infty;
      const double rhoE_fac = wave_amps[w]/gamma_m_1;
      const double omt = wave_omegas[w]*t;

      const std::size_t w_offset = w*num_pts;

      for (std::size_t i = 0; i < num_pts; i++)
      {
         const double cos_w = std::cos(k_dot_x_p_phi[w_offset + i] - omt);

         rho[i] += rho_fac*cos_w;

         rhoV[i] += rhoV_fac*cos_w*mod_wave_dirs[w];
         if constexpr(TDim > 1)
         {
            rhoV[num_pts + i] += rhoV_fac*cos_w*mod_wave_dirs[num_waves + w];
         }
         if constexpr(TDim > 2)
         {
            rhoV[2*num_pts + i] += rhoV_fac*cos_w*mod_wave_dirs[2*num_waves + w];
         }
         rhoE[i] += rhoE_fac*cos_w;

      }
   }

   for (std::size_t i = 0; i < num_pts; i++)
   {
      double mag_u = 0.0;
      
      const double val0 = rhoV[i];
      mag_u += val0*val0;
      if constexpr (TDim > 1)
      {
         const double val1 = rhoV[num_pts + i];
         mag_u += val1*val1;
      }
      if constexpr (TDim > 2)
      {
         const double val2 = rhoV[2*num_pts + i];
         mag_u += val2*val2;
      }
      rhoE[i] += 0.5*rho[i]*mag_u;
   }
   
   for (std::size_t i = 0; i < num_pts; i++)
   {
      rhoV[i] *= rho[i];
      if constexpr (TDim > 1)
      {
         rhoV[num_pts + i] *= rho[i];
      }
      if constexpr (TDim > 2)
      {
         rhoV[2*num_pts + i] *= rho[i];
      }
   }
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
