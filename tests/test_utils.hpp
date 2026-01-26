#ifndef JABBER_TEST_UTILS
#define JABBER_TEST_UTILS

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

template<std::size_t Size>
std::vector<double> GenerateRandomVec(int seed, double s0, double s1)
{
   std::array<double, Size> temp = GenerateRandomArr<Size>(seed, s0, s1);
   return std::vector<double>(temp.begin(), temp.end());
}

} // namespace jabber_test

#endif // JABBER_TEST_UTILS
