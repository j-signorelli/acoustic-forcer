#include "acoustic_field.hpp"

namespace jabber
{

AcousticField::AcousticField(int dim, const std::vector<double> coords)
: dim_(dim),
  coords_(dim),
  wave_number_(dim)
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

   for (std::vector<double> &wave_number_d : wave_number_)
   {
      wave_number_d.resize(num_waves);
   }
   num_waves_ = num_waves;
}

} // namespace jabber
