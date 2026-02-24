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


/// Generate a random OptionEnum.
template<OptionEnum T>
T GenerateRandomOption(int seed)
{
   std::mt19937 gen(seed);
   std::uniform_int_distribution<int> int_dist(0, static_cast<int>(T::Size)-1);
   return static_cast<T>(int_dist(gen));
}

#ifdef JABBER_WITH_APP

/**
 * @brief Generate completely randomized input xy params type. Parameters that
 * vary should be manually updated for comprehensive testing.
 */
jabber_app::InputXYParamsVariant GenerateRandomInputXY(
                                    jabber_app::InputXYOption f, int seed);

/**
 * @brief Generate completely randomized function params type. Parameters that
 * vary should be manually updated for comprehensive testing.
 */
jabber_app::FunctionParamsVariant GenerateRandomFunction(
                                    jabber_app::FunctionOption f, int seed);

/**
 * @brief Generate completely randomized disc. method params type. Parameters that
 * vary should be manually updated for comprehensive testing.
 */
jabber_app::DiscMethodParamsVariant GenerateRandomDiscMethod(
                                    jabber_app::DiscMethodOption d, int seed);

/**
 * @brief Generate completely randomized direction params type. Parameters that
 * vary should be manually updated for comprehensive testing.
 */
jabber_app::DirectionParamsVariant GenerateRandomDirection(
                                    jabber_app::DirectionOption d, int seed);

/**
 * @brief Generate completely randomized source params type. Parameters that
 * vary should be manually updated for comprehensive testing.
 */
jabber_app::SourceParamsVariant GenerateRandomSource(
                                    jabber_app::SourceOption s, int seed);


#endif // JABBER_WITH_APP

} // namespace jabber_test

#endif // JABBER_TEST_UTILS
