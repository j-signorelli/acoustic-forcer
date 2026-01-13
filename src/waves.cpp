#include "waves.hpp"

namespace jabber
{

Waves::Waves(int dim, int num_waves)
: dim_(dim)
{
   wave_number_.resize(dim);
   SetNumWaves(num_waves);
}

void Waves::SetNumWaves(int num_waves)
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
