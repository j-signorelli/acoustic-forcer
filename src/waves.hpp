#ifndef JABBER_WAVE
#define JABBER_WAVE

#include <vector>

namespace jabber
{

/// Struct-of-arrays class for all waves
class Waves
{
private:
   const int dim_;
   int num_waves_;

   std::vector<double> amplitude_;
   std::vector<double> frequency_;
   std::vector<double> phase_;
   std::vector<std::vector<double>> wave_number_;
   
public:
   Waves(int dim_, int num_waves=0);

   int Dim() const { return dim_; }

   void SetNumWaves(int num_waves);

   int NumWaves() const { return num_waves_; }

   double& Amplitude(int i) { return amplitude_[i]; }
   const double& Amplitude(int i) const { return amplitude_[i]; }

   double& Frequency(int i) { return frequency_[i]; }
   const double& Frequency(int i) const { return frequency_[i]; }

   double& Phase(int i) { return phase_[i]; }
   const double& Phase(int i) const { return phase_[i]; }

   double& WaveNumber(int d, int i) { return wave_number_[d][i]; }
   const double& WaveNumber(int d, int i) const { return wave_number_[d][i]; }
};

} // namespace jabber

#endif // JABBER_WAVE
