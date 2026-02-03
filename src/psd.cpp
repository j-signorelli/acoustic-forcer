#include "psd.hpp"

#include <cmath>

namespace jabber
{

void DiscretizePSDRiemann(std::function<double(double)> psd,
                           std::span<const double> freqs,
                           std::span<double> powers,
                           bool log_scale)
{
   // Define lambda fxn for computing Δf
   auto compute_df = 
   [log_scale](const double fkm1, const double fkp1)
   {
      if (log_scale)
      {
         double log_fkm1 = std::log10(fkm1);
         double log_fkp1 = std::log10(fkp1);
         return std::pow(10, (log_fkp1-log_fkm1)/2);
      }
      else
      {
         return (fkp1-fkm1)/2;
      }
   };

   // Compute all interior powers
   const std::size_t N = freqs.size()-1;
   for (std::size_t i = 1; i < N; i++)
   {  
      powers[i] = psd(freqs[i])*compute_df(freqs[i-1], freqs[i+1]);
   }

   //  Handle boundaries
   powers[0] = psd(freqs[0])*(2*compute_df(freqs[1], freqs[0]));
   powers[N] = psd(freqs[N])*(2*compute_df(freqs[N-1], freqs[N]));
}

} // namespace jabber
