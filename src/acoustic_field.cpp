#include "acoustic_field.hpp"

#include <math.h>

namespace jabber
{

AcousticField::AcousticField(int dim, const std::vector<double> coords)
: dim_(dim),
  num_pts(dim_*coords.size()),
  coords_(dim_),
  k_(dim)
{
   // Store the coordinates in an SoA-style
   for (int d = 0; d < dim_; d++)
   {
      coords_[d].resize(num_pts);
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
   k_dot_x_p_phi_.resize(NumWaves(), 0.0);
   omega_.resize(NumWaves());

   // Compute k·x+φ
   for (int d = 0; d < Dim(); d++)
   {
      for (int i = 0; i < NumWaves(); i++)
      {
         k_dot_x_p_phi_[i] += k_[d][i]*coords_[d][i];
      }
   }

   // Compute ω=2πf
   for (int i = 0; i < NumWaves(); i++)
   {
      omega_[i] += 2*M_PI*frequency_[i];
   }
}

void AcousticField::Compute(double t, std::vector<double> &p_prime) const
{
   // Compute p'=cos(k·x+φ+ωt)
   for (int i = 0; i < NumWaves(); i++)
   {
      p_prime[i] = amplitude_[i]*std::cos(k_dot_x_p_phi_[i]+omega_[i]*t);
   }
}

} // namespace jabber
