#ifndef JABBER_TEST_UTILS
#define JABBER_TEST_UTILS

#include <random>

namespace jabber_test
{

/**
 * @brief Generate std::array of random doubles in range [ \p s0 , \p s1 ],
 * using seed \p seed.
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

} // namespace jabber_test

#endif // JABBER_TEST_UTILS
