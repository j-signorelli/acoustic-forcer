#include "acoustic_field.hpp"
#include "kernels.hpp"

#include <math.h>
#include <numeric>
#include <iostream>
#include <format>
#include <string>
#include <ranges>

namespace jabber
{

void WriteWaves(std::span<const Wave> waves, std::ostream &out)
{
   for (std::size_t i = 0; i < waves.size(); i++)
   {
      out << std::format("{},{},{},{},", waves[i].amplitude,
                                          waves[i].frequency,
                                          waves[i].phase, 
                                          waves[i].speed);
      for (std::size_t d = 0; d < waves[i].k_hat.size(); d++)
      {
         out << std::format("{},", waves[i].k_hat[d]);
         out << (d+1==waves[i].k_hat.size() ? "\n" : "");
      }
   }
}

void ReadWaves(std::istream &in, std::vector<Wave> &waves)
{
   for (std::string line; std::getline(in, line);)
   {
      auto field_view = std::ranges::views::split(line, 
                                                   std::string_view(","));
      auto range_it = field_view.begin();
      auto field_it = *range_it;

      Wave w;
      w.amplitude = std::stod(std::string(field_it.begin(), field_it.end()));
      field_it = *(++range_it);

      w.frequency = std::stod(std::string(field_it.begin(), field_it.end()));
      field_it = *(++range_it);

      w.phase = std::stod(std::string(field_it.begin(), field_it.end()));
      field_it = *(++range_it);

      w.speed = std::string(field_it.begin(), field_it.end())[0];
      field_it = *(++range_it);
      
      // Continue parsing until empty string (marking end of row)
      while (!field_it.empty())
      {
         const double val = std::stod(std::string(field_it.begin(), 
                                                   field_it.end()));
         w.k_hat.push_back(val);
         field_it = *(++range_it);
      }
      waves.emplace_back(w);
   }
}

AcousticField::AcousticField(int dim, std::span<const double> coords,
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
   omega_.resize(NumWaves());
   mod_k_hat_.resize(Dim()*NumWaves());
   k_dot_x_p_phi_.resize(NumWaves()*NumPoints());

   // Note that performance of below was not carefully considered
   for (int w = 0; w < NumWaves(); w++)
   {
      const Wave &wave = Waves()[w];

      // Set amplitude
      amplitude_[w] = wave.amplitude;

      // Compute + set ω=2πf
      omega_[w] = 2*M_PI*wave.frequency;

      // Compute U·k_hat±c and set mod_k_hat_
      double denom = (wave.speed == 'S' ? -c_infty_ : c_infty_);
      double speed_encoder = (wave.speed == 'S' ? -1 : 1);
      for (int d = 0; d < Dim(); d++)
      {
         denom += U_bar_[d]*wave.k_hat[d];
         mod_k_hat_[d*NumWaves() + w] = wave.k_hat[d]*speed_encoder;
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
                        mod_k_hat_.data(), k_dot_x_p_phi_.data(), t, rho_.data(),
                        rhoV_.data(), rhoE_.data());
   }
   else if (Dim() == 2)
   {
      ComputeKernel<2>(NumPoints(), rho_bar_, p_bar_, U_bar_.data(), gamma_,
                        NumWaves(), amplitude_.data(), omega_.data(),
                        mod_k_hat_.data(), k_dot_x_p_phi_.data(), t, rho_.data(),
                        rhoV_.data(), rhoE_.data());
   }
   else
   {
      ComputeKernel<3>(NumPoints(), rho_bar_, p_bar_, U_bar_.data(), gamma_,
                        NumWaves(), amplitude_.data(), omega_.data(),
                        mod_k_hat_.data(), k_dot_x_p_phi_.data(), t, rho_.data(),
                        rhoV_.data(), rhoE_.data());
   }
}



} // namespace jabber
