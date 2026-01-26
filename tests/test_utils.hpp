#ifndef JABBER_TEST_UTILS
#define JABBER_TEST_UTILS

#ifdef JABBER_WITH_APP
#include <jabber_app.hpp>
#endif // JABBER_WITH_APP
#include <random>

namespace jabber_test
{

/// Generate random double within \p s0 and \p s1 using \p seed.
inline double GenerateRandomReal(int seed, double s0, double s1)
{
   std::mt19937 gen(seed);
   std::uniform_real_distribution<double> real_dist(s0, s1);
   return real_dist(gen);
}
/**
 * @brief Generate std::array of random doubles within \p s0 and \p s1 
 * using \p seed.
 */
template<std::size_t Size>
std::array<double, Size> GenerateRandomArr(int seed, double s0, double s1)
{
   std::array<double, Size> dat;
   std::mt19937 gen(seed);

   std::uniform_real_distribution<double> real_dist_x(s0, s1);
   for (std::size_t i = 0; i < Size; i++)
   {
      dat[i] = real_dist_x(gen);
   }
   return dat;
}

/**
 * @brief Generate std::vector of random doubles within \p s0 and \p s1 
 * using \p seed.
 */
template<std::size_t Size>
std::vector<double> GenerateRandomVec(int seed, double s0, double s1)
{
   std::array<double, Size> temp = GenerateRandomArr<Size>(seed, s0, s1);
   return std::vector<double>(temp.begin(), temp.end());
}

#ifdef JABBER_WITH_APP
/**
 * @brief Generate (largely) randomized source params type.
 */
template<jabber_app::SourceOption s>
inline jabber_app::SourceParams<s> GenerateRandomSource(int seed)
{
   using namespace jabber_app;
   if constexpr (s == SourceOption::SingleWave)
   {
      SourceParams<SourceOption::SingleWave> wave;
      wave.amp = GenerateRandomReal(seed++,0.1,10.0);
      wave.freq = GenerateRandomReal(seed++, 500.0, 1500.0);
      wave.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
      wave.phase = GenerateRandomReal(seed++, 10.0, 180.0);
      wave.speed = (seed % 2 == 0 ? SpeedOption::Slow
                                  : SpeedOption::Fast);
      return wave;
   }
   else if constexpr (s == SourceOption::WaveSpectrum)
   {
      constexpr int kNumWaves = 5;
      SourceParams<SourceOption::WaveSpectrum> waves;
      waves.amps = GenerateRandomVec<kNumWaves>(seed++, 1.0, 10.0);
      waves.freqs = GenerateRandomVec<kNumWaves>(seed++, 500.0, 
                                                            1500.0);
      waves.directions.resize(kNumWaves);
      for (std::vector<double> &w_dir : waves.directions)
      {
            w_dir = GenerateRandomVec<3>(seed++, 0.0, 1.0);
      }
      waves.phases = GenerateRandomVec<kNumWaves>(seed++,0.0,180.0);
      waves.speeds.resize(kNumWaves);
      for (int w = 0; w < kNumWaves; w++)
      {
            waves.speeds[w] = (w%2==0 ? SpeedOption::Slow
                                    : SpeedOption::Fast);
      }
      return waves;
   }
   else
   {
      static_assert("Type not implemented.");
   }
}
#endif // JABBER_WITH_APP

} // namespace jabber_test

#endif // JABBER_TEST_UTILS
