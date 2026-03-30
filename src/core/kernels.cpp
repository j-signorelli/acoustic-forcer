#include "kernels.hpp"

#include <cmath>
#include <memory>

namespace jabber
{

template<std::size_t TDim, bool TGridInnerLoop>
void ComputeKernel(const std::size_t num_pts, const double rho_bar,
                        const double p_bar, const double *U_bar, 
                        const double gamma, const int num_waves,
                        const double t,
                        const double *__restrict__ rho_coeffs,
                        const double *__restrict__ rhoV_coeffs,
                        const double *__restrict__ rhoE_coeffs, 
                        const double *__restrict__ wave_omegas,
                        const double *__restrict__ k_dot_x_p_phi,
                        double *__restrict__ rho,
                        double *__restrict__ rhoV,
                        double *__restrict__ rhoE)
{
 
   // Explicitly tell compiler to assume memory is aligned
   const double *__restrict__ a_rho_coeffs = std::assume_aligned<MEMALIGN_BYTES>(rho_coeffs);
   const double *__restrict__ a_rhoV_coeffs = std::assume_aligned<MEMALIGN_BYTES>(rhoV_coeffs);
   const double *__restrict__ a_rhoE_coeffs = std::assume_aligned<MEMALIGN_BYTES>(rhoE_coeffs);
   const double *__restrict__ a_wave_omegas = std::assume_aligned<MEMALIGN_BYTES>(wave_omegas);
   const double *__restrict__ a_k_dot_x_p_phi = std::assume_aligned<MEMALIGN_BYTES>(k_dot_x_p_phi);
   double *__restrict__ a_rho = std::assume_aligned<MEMALIGN_BYTES>(rho);
   double *__restrict__ a_rhoV = std::assume_aligned<MEMALIGN_BYTES>(rhoV);
   double *__restrict__ a_rhoE = std::assume_aligned<MEMALIGN_BYTES>(rhoE);

   const double rhoE_init = p_bar/(gamma-1.0);

   if constexpr (TGridInnerLoop)
   {
      for (std::size_t i = 0; i < num_pts; i++)
      {
         a_rho[i] = rho_bar;

         a_rhoV[i] = U_bar[0];
         if constexpr(TDim > 1)
         {
            a_rhoV[num_pts + i] = U_bar[1];
         }
         if constexpr(TDim > 2)
         {
            a_rhoV[2*num_pts + i] = U_bar[2];
         }
         a_rhoE[i] = rhoE_init;
      }

      // Add contribution of each wave
#ifdef JABBER_WITH_OPENMP
      #pragma omp parallel for reduction(+:rho[0:num_pts],\
                                          rhoV[0:num_pts*TDim],\
                                          rhoE[0:num_pts])
#endif // JABBER_WITH_OPENMP
      for (int w = 0; w < num_waves; w++)
      {
         const double rho_coeff_w = a_rho_coeffs[w];
         const double rhoV1_coeff_w = a_rhoV_coeffs[w];
         const double rhoV2_coeff_w = TDim > 1 ? a_rhoV_coeffs[num_waves + w] : 0;
         const double rhoV3_coeff_w = TDim > 2 ? a_rhoV_coeffs[2*num_waves + w] : 0;
         const double rhoE_coeff_w = a_rhoE_coeffs[w];
         const double omt = a_wave_omegas[w]*t;

         const std::size_t w_offset = w*num_pts;

         for (std::size_t i = 0; i < num_pts; i++)
         {
            const double cos_w = std::cos(a_k_dot_x_p_phi[w_offset + i] - omt);

            a_rho[i] += rho_coeff_w*cos_w;
            a_rhoV[i] += rhoV1_coeff_w*cos_w;
            if constexpr(TDim > 1)
            {
               a_rhoV[num_pts + i] += rhoV2_coeff_w*cos_w;
            }
            if constexpr(TDim > 2)
            {
               a_rhoV[2*num_pts + i] += rhoV3_coeff_w*cos_w;
            }
            a_rhoE[i] += rhoE_coeff_w*cos_w;

         }
      }
   }
   else
   {
#ifdef JABBER_WITH_OPENMP
      #pragma omp parallel for
#endif // JABBER_WITH_OPENMP
      for (std::size_t i = 0; i < num_pts; i++)
      {
         double rho_i = rho_bar;
         double rhoV1_i = U_bar[0];
         double rhoV2_i = TDim > 1 ? U_bar[1] : 0.0;
         double rhoV3_i = TDim > 2 ? U_bar[2] : 0.0;
         double rhoE_i = rhoE_init;

         const std::size_t i_offset = i*num_waves;
         
         for (int w = 0; w < num_waves; w++)
         {
            const double omt = a_wave_omegas[w]*t;
            const double cos_w = std::cos(a_k_dot_x_p_phi[i_offset + w] - omt);

            rho_i += a_rho_coeffs[w]*cos_w;
            rhoV1_i += a_rhoV_coeffs[w]*cos_w;
            if constexpr (TDim > 1)
            {
               rhoV2_i += a_rhoV_coeffs[num_waves + w]*cos_w;
            }
            if constexpr (TDim > 2)
            {
               rhoV3_i += a_rhoV_coeffs[2*num_waves + w]*cos_w;
            }
            rhoE_i += a_rhoE_coeffs[w]*cos_w;
         }
         a_rho[i] = rho_i;
         a_rhoV[i] = rhoV1_i;
         if constexpr (TDim > 1)
         {
            a_rhoV[num_pts + i] = rhoV2_i;
         }
         if constexpr (TDim > 2)
         {
            a_rhoV[2*num_pts + i] = rhoV3_i;
         }
         a_rhoE[i] = rhoE_i;
      }
   }

   for (std::size_t i = 0; i < num_pts; i++)
   {
      double mag_u = 0.0;
      
      const double val0 = a_rhoV[i];
      mag_u += val0*val0;
      if constexpr (TDim > 1)
      {
         const double val1 = a_rhoV[num_pts + i];
         mag_u += val1*val1;
      }
      if constexpr (TDim > 2)
      {
         const double val2 = a_rhoV[2*num_pts + i];
         mag_u += val2*val2;
      }
      a_rhoE[i] += 0.5*a_rho[i]*mag_u;
   }
   
   for (std::size_t i = 0; i < num_pts; i++)
   {
      a_rhoV[i] *= a_rho[i];
      if constexpr (TDim > 1)
      {
         a_rhoV[num_pts + i] *= a_rho[i];
      }
      if constexpr (TDim > 2)
      {
         a_rhoV[2*num_pts + i] *= a_rho[i];
      }
   }
}

// Explicit instantiation for Dims 1-3
template void ComputeKernel<1, true>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);
                                
template void ComputeKernel<2, true>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);

template void ComputeKernel<3, true>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);

template void ComputeKernel<1, false>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);
                                
template void ComputeKernel<2, false>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);

template void ComputeKernel<3, false>(const std::size_t, const double,
                                 const double, const double *, 
                                 const double, const int, const double,
                                 const double *__restrict__, 
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 const double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__,
                                 double *__restrict__);


} // namespace jabber
