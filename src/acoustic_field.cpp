#include "acoustic_field.hpp"

#include <math.h>

namespace jabber
{

AcousticField::AcousticField(int dim, const std::vector<double> coords)
: dim_(dim),
  num_pts_(coords.size()/dim_),
  coords_(dim_),
  k_(dim)
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

void AcousticField::SetNumWaves(int num_waves)
{
   amplitude_.resize(num_waves);
   frequency_.resize(num_waves);
   phase_.resize(num_waves);

   for (std::vector<double> &k_d : k_)
   {
      k_d.resize(num_waves);
   }
   num_waves_ = num_waves;
}

void AcousticField::ReserveNumWaves(int num_waves)
{
   amplitude_.reserve(num_waves);
   frequency_.reserve(num_waves);
   phase_.reserve(num_waves);

   for (std::vector<double> &k_d : k_)
   {
      k_d.reserve(num_waves);
   }
}

void AcousticField::AddWave(const Wave &w)
{
   amplitude_.push_back(w.amplitude);
   frequency_.push_back(w.frequency);
   phase_.push_back(w.phase);

   for (int d = 0; d < Dim(); d++)
   {
      k_[d].push_back(w.k[d]);
   }
   num_waves_++;
}

void AcousticField::GetWave(int i, Wave &w) const
{
   w.amplitude = Amplitude(i);
   w.frequency = Frequency(i);
   w.phase = Phase(i);
   w.k.resize(Dim());
   for (int d = 0; d < Dim(); d++)
   {
      w.k[d] = WaveNumber(d, i);
   }
}

void AcousticField::Finalize()
{
   k_dot_x_p_phi_.resize(NumWaves()*NumPoints(), 0.0);
   omega_.resize(NumWaves());

   // Compute k·x+φ
   for (int w = 0; w < NumWaves(); w++)
   {
      std::size_t w_offset = w*NumPoints();
      for (int d = 0; d < Dim(); d++)
      {
         for (std::size_t i = 0; i < NumPoints(); i++)
         {
            k_dot_x_p_phi_[w_offset + i] += k_[d][w]*coords_[d][i];
         }
      }
   }

   // Compute ω=2πf
   for (int w = 0; w < NumWaves(); w++)
   {
      omega_[w] = 2*M_PI*frequency_[w];
   }
}

void AcousticField::Compute(double t, std::vector<double> &p_prime) const
{
   p_prime.assign(NumPoints(), 0.0);

   // Compute p'=cos(k·x+φ-ωt)
   for (int w = 0; w < NumWaves(); w++)
   {
      double amp = amplitude_[w];
      double omt = omega_[w]*t;
      std::size_t w_offset = w*NumPoints();
      for (std::size_t i = 0; i < NumPoints(); i++)
      {
         p_prime[i] += amp*std::cos(k_dot_x_p_phi_[w_offset + i] - omt);
      }
   }
}

} // namespace jabber
