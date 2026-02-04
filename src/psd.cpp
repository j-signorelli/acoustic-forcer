#include "psd.hpp"

#include <cmath>

namespace jabber
{

void DiscretizePSDRiemann(std::span<const double> freqs,
                           std::span<const double> psd,
                           std::span<double> powers,
                           bool log_scale)
{
   const std::size_t N = freqs.size()-1;

   if (!log_scale)
   {
      // Compute all interior powers
      for (std::size_t i = 1; i < N; i++)
      {  
         powers[i] = psd[i]*(freqs[i+1]-freqs[i-1])/2.0;
      }

      //  Handle boundaries
      powers[0] = psd[0]*(freqs[1]-freqs[0])/2.0;
      powers[N] = psd[N]*(freqs[N]-freqs[N-1])/2.0;
   }
   else
   {
      // Can be shown that log10 midpoint of f_1 and f_2 is sqrt(f_1*f_2)

      // Compute all interior powers
      for (std::size_t i = 1; i < N; i++)
      {  
         const double df = std::sqrt(freqs[i]*freqs[i+1]) 
                           - std::sqrt(freqs[i]*freqs[i-1]);
         powers[i] = psd[i]*df;
      }
      
      //  Handle boundaries
      powers[0] = psd[0]*(std::sqrt(freqs[0]*freqs[1]) - freqs[0]);
      powers[N] = psd[N]*(freqs[N] - std::sqrt(freqs[N]*freqs[N-1]));
   }
}

} // namespace jabber
