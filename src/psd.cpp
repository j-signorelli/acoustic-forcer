#include "psd.hpp"

#include <cmath>
#include <stdexcept>

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

PWLogLogPSD::PWLogLogPSD(std::span<const double> freq, 
                           std::span<const double> psd)
{
   for (std::size_t i = 0; i < freq.size(); i++)
   {
      freq_psd_map_.insert({freq[i], psd[i]});
   }
}

double PWLogLogPSD::operator() (double f) const
{
   auto it = freq_psd_map_.upper_bound(f);

   // Extrapolate from first/last interior curve
   //  if f falls below/above min/max frequency
   if (it == freq_psd_map_.begin())
   {
      it++;
   }
   if (it == freq_psd_map_.end())
   {
      it--;
   }

   const double f_2 = it->first;
   const double psd_2 = it->second;
   it--;
   const double f_1 = it->first;
   const double psd_1 = it->second;

   const double m = (std::log(psd_2/psd_1))/(std::log(f_2/f_1));

   return psd_1*std::pow((f/f_1), m);
}

void PWLogLogPSD::Discretize(std::span<const double> freqs, 
                              std::span<double> powers) const
{
   
}

} // namespace jabber
