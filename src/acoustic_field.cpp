#include "acoustic_field.hpp"

#include <math.h>
#include <numeric>

namespace jabber
{

template<int TDim>
void AcousticField::ComputeKernel(const std::size_t num_pts, const double rho_bar,
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
   for (int w = 0; w < NumWaves(); w++)
   {
      const double rho_fac = wave_amps[w]/(c_infty*c_infty);
      const double rhoV_fac = wave_amps[w]/(rho_bar*c_infty);
      const double rhoE_fac = wave_amps[w]/(gamma-1.0);

      const double omt = wave_omegas[w]*t;
      const std::size_t w_offset = w*num_pts;

      // THIS SHOULD BE VECTORIZED:
      for (std::size_t i = 0; i < num_pts(); i++)
      {
         double cos_w = std::cos(k_dot_x_p_phi[w_offset + i] - omt);

         // Add: \rho += (1/c_\infty^2)p'_w*cos(...)
         rho_[i] += rho_fac*cos_w;

         // Unroll to ensure that loop over points (inner-most) is
         // vectorized, NOT over these dims.
         #pragma unroll
         for (std::size_t d = 0; d < TDim; d++)
         {
            // Add: \rhoV += (1/(\bar{\rho}))
            rhoV_[d*num_pts + i] += rhoV_fac*cos_w*k_hat[d*num_waves + w];
         }

         rhoE_[i] += rhoE_fac*cos_w;

      }
   }

   // TODO: One more loop to postprocess (multiply rhoV by rho, do energy shit, etc.)

}


AcousticField::AcousticField(int dim, const std::vector<double> coords,
                  double p_bar, double rho_bar,
                  const std::vector<double> U_bar, double gamma)
: dim_(dim),
  num_pts_(coords.size()/dim_),
  p_bar_(p_bar),
  rho_bar_(rho_bar),
  U_bar_(U_bar),
  gamma_(gamma),
  c_infty_(std::sqrt(gamma_*p_bar_/rho_bar_)),
  coords_(dim_)
{
   
   // Store the coordinates in an SoA-style
   for (int d = 0; d < Dim(); d++)
   {
      coords_[d].resize(NumPoints());
      for (int i = 0; i < NumPoints(); i++)
      {
         coords_[d][i] = coords[i*Dim() + d];
      }
   }
}

void AcousticField::Finalize()
{
   // Allocate non-time-varying constants
   amplitude_.resize(NumWaves());
   k_dot_x_p_phi_.resize(NumWaves()*NumPoints(), 0.0);
   omega_.resize(NumWaves());

   // Note that performance of below was not carefully considered
   for (int w = 0; w < NumWaves(); w++)
   {
      const Wave &wave = GetWave(w);

      // Set amplitude
      amplitude_[w] = wave.amplitude;

      // Compute + set ω=2πf
      omega_[w] = 2*M_PI*wave.frequency;

      // Compute U·k_hat±c
      double denom = wave.speed_flag ? -c_infty_ : c_infty_;
      for (int d = 0; d < Dim(); d++)
      {
         denom += U_bar_[d]*wave.k_hat[d];
      }

      // Compute magnitude of wavelength vector k
      double k = omega_[w]/denom;

      // Compute + set k·x+φ
      std::size_t w_offset = w*NumPoints();
      for (std::size_t i = 0; i < NumPoints(); i++)
      {  
         k_dot_x_p_phi_[w_offset + i] = wave.phase;
         for (int d = 0; d < Dim(); d++)
         {
            k_dot_x_p_phi_[w_offset + i] += wave.k_hat[d]*k*coords_[d][i];
         }
      }
   }

   // Allocate flow solution memory
   rho_.resize(NumPoints());
   rhoV_.resize(NumPoints()*Dim());
   rhoE_.resize(NumPoints());
}

void AcousticField::Compute(double t)
{
   // Dispatch to appropriate kernel
   if (Dim() == 1)
   {
      ComputeKernel<1>(NumPoints(), rho_bar_, p_bar_, U_bar_.data(), gamma_,
                        NumWaves(), amplitude_.data(), omega_.data(), 
                        k_dot_x_p_phi_.data(), t, rho_.data(), rhoV_.data(), 
                        rhoE_.data());
   }
   else if (Dim() == 2)
   {
      ComputeKernel<2>(NumPoints(), rho_bar_, p_bar_, U_bar_.data(), gamma_,
                        NumWaves(), amplitude_.data(), omega_.data(), 
                        k_dot_x_p_phi_.data(), t, rho_.data(), rhoV_.data(), 
                        rhoE_.data());
   }
   else
   {
      ComputeKernel<3>(NumPoints(), rho_bar_, p_bar_, U_bar_.data(), gamma_,
                        NumWaves(), amplitude_.data(), omega_.data(), 
                        k_dot_x_p_phi_.data(), t, rho_.data(), rhoV_.data(), 
                        rhoE_.data());
   }
}



} // namespace jabber
