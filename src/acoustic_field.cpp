#include "acoustic_field.hpp"

namespace jabber
{

AcousticField::AcousticField(int dim, const std::vector<double> coords)
: dim_(dim),
  coords_(dim),
  k_(dim)
{
   // Store the coordinates in an SoA-style
   int num_pts = coords.size()/dim_;
   for (int d = 0; d < dim_; d++)
   {
      coords_[d].resize(num_pts);
      for (int i = 0; i < num_pts; i++)
      {
         coords_[d][i] = coords[i*dim_ + d];
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
   
}

} // namespace jabber
