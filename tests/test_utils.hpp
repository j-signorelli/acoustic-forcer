#ifndef JABBER_TEST_UTILS
#define JABBER_TEST_UTILS

#include <catch2/generators/catch_generators.hpp>

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

/// Concept for enums of type std::uint8_t with a ::Size.
template<typename T>
concept OptionEnum = 
         std::is_enum_v<T> && 
         std::same_as<std::underlying_type_t<T>, std::uint8_t> &&
         requires {T::Size; };

/// Custom generator for \ref OptionEnum.
template<OptionEnum T>
class OptionGenerator : public Catch::Generators::IGenerator<T>
{
private:
   T enumerator_val_;
   std::uint8_t enum_idx_ = 0;
public:
   bool next() override
   {
      if (++enum_idx_ < static_cast<std::uint8_t>(T::Size))
      {
         enumerator_val_ = static_cast<T>(enum_idx_);
         return true;
      }
      else
      {
         return false;
      }
   };

   T const& get() const override
   {
      return enumerator_val_;
   }
};

template <OptionEnum T>
Catch::Generators::GeneratorWrapper<T> options()
{
    return Catch::Generators::GeneratorWrapper<T>(
               Catch::Detail::make_unique<OptionGenerator<T>>());
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
      wave.speed = (seed % 2 == 0 ? 'S' : 'F');
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
            waves.speeds[w] = (w%2==0 ? 'S' : 'F');
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
