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

/// Generate random integer within \p s0 and \p s1 using \p seed.
inline int GenerateRandomInt(int seed, int s0, int s1)
{
   std::mt19937 gen(seed);
   std::uniform_int_distribution<int> int_dist(s0, s1);
   return int_dist(gen);
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
template<jabber_app::SourceOption s, typename... SubOptions>
jabber_app::SourceParams<s> GenerateRandomSource(int seed, SubOptions... opts);

template<>
jabber_app::SourceParams<jabber_app::SourceOption::SingleWave>
GenerateRandomSource<jabber_app::SourceOption::SingleWave>(int seed,
                                                            char speed);

template<>
jabber_app::SourceParams<jabber_app::SourceOption::WaveSpectrum>
GenerateRandomSource<jabber_app::SourceOption::WaveSpectrum>(int seed);

template<>
jabber_app::SourceParams<jabber_app::SourceOption::DigitalPSD>
GenerateRandomSource<jabber_app::SourceOption::DigitalPSD>(int seed,
                       jabber_app::InterpolationOption interp_method,
                       jabber::Interval::Method int_method,
                       jabber_app::DiscMethodOption disc_method,
                       jabber_app::DirectionOption dir_method,
                       char speed);

#endif // JABBER_WITH_APP

} // namespace jabber_test

#endif // JABBER_TEST_UTILS
