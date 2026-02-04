#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <jabber.hpp>

#include <array>

using namespace jabber;
using namespace Catch::Matchers;

namespace jabber_test
{

TEST_CASE("DiscretizePSDRiemann", "[PSD] [PSDQuadrature]")
{
   constexpr std::array<double, 5> freqs = {0.1e3, 1e3, 10e3, 100e3, 200e3};
   constexpr std::array<double, 5> psd =   {1e-8, 5e-9, 2e-9, 1e-10, 2e-12};

   std::vector<double> powers_test(5);
   
   SECTION("Standard Midpoint")
   {
      const std::vector<double> powers_exact({4.5e-6, 2.475e-5, 9.9e-5,
                                                               9.5e-6, 1e-7});

      DiscretizePSDRiemann(freqs, psd, powers_test);

      for (std::size_t i = 0; i < 5; i++)
      {
         CHECK_THAT(powers_exact[i], WithinULP(powers_test[i], 1));
      }
   }

   SECTION("Logarithmic Midpoint")
   {
      const std::vector<double> powers_exact({2.1622776601683794e-06,
                                              1.4230249470757709e-05,
                                              5.692099788303083e-05,
                                              1.0979857963562574e-05,
                                              1.1715728752538096e-07});

      DiscretizePSDRiemann(freqs, psd, powers_test, true);

      for (std::size_t i = 0; i < 5; i++)
      {
         CHECK_THAT(powers_exact[i], WithinULP(powers_test[i], 1));
      }
   }

}

} // namespace jabber_test
