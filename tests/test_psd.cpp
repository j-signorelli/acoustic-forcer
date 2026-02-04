#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <jabber.hpp>

#include <array>

using namespace jabber;
using namespace Catch::Matchers;

namespace jabber_test
{

static constexpr std::array<double, 5> kFreqs={0.1e3, 1e3, 10e3, 100e3, 200e3};
static constexpr std::array<double, 5> kPsd={1e-8, 5e-8, 7e-9, 1e-10, 2e-12};

TEST_CASE("DiscretizePSDRiemann", "[PSD] [PSDQuadrature]")
{
   std::vector<double> powers_test(5);
   
   SECTION("Standard Midpoint")
   {
      const std::vector<double> powers_exact({4.5e-6, 0.0002475, 0.0003465, 
                                                   9.5e-6, 1e-7});

      DiscretizePSDRiemann(kFreqs, kPsd, powers_test);

      for (std::size_t i = 0; i < 5; i++)
      {
         CAPTURE(i);
         CHECK_THAT(powers_test[i], WithinULP(powers_exact[i], 1));
      }
   }

   SECTION("Logarithmic Midpoint")
   {
      const std::vector<double> powers_exact({2.1622776601683794e-06,
                                              0.00014230249470757706,
                                              0.00019922349259060787,
                                              1.0979857963562574e-05,
                                              1.1715728752538096e-07});

      DiscretizePSDRiemann(kFreqs, kPsd, powers_test, true);

      for (std::size_t i = 0; i < 5; i++)
      {
         CAPTURE(i);
         CHECK_THAT(powers_test[i], WithinULP(powers_exact[i], 1));
      }
   }
}

TEST_CASE("PWLogLogPSD::operator()", "[PSD] [PSDInterpolant] [PWLogLogPSD]")
{
   PWLogLogPSD psd_interp(kFreqs, kPsd);
   constexpr std::array<double, 6> sample_freqs = {0.1e3, 0.75e3, 
                                                   5.5e3, 10e3, 150e3, 200e3};
   constexpr std::array<double, 6> sample_psd_exact = {1e-8,
                                                       4.0892303464925605e-08,
                                                       1.166259140008537e-08,
                                                       7e-9,
                                                       1.0142996079328494e-11,
                                                       2e-12};

   for (std::size_t i = 0; i < 6; i++)
   {
      CAPTURE(i);
      CHECK_THAT(psd_interp(sample_freqs[i]), 
                              WithinULP(sample_psd_exact[i], 1));
   }
}

TEST_CASE("PWLogLogPSD::Discretize", "[PSD] [PSDInterpolant] [PWLogLogPSD]")
{
   PWLogLogPSD psd_interp(kFreqs, kPsd);
   constexpr std::array<double, 6> sample_freqs = {0.1e3, 0.75e3, 
                                                   5.5e3, 10e3, 150e3, 200e3};
   constexpr std::array<double, 6> sample_psd_exact = {1e-8,
                                                       4.0892303464925605e-08,
                                                       1.166259140008537e-08,
                                                       7e-9,
                                                       1.0142996079328494e-11,
                                                       2e-12};

   for (std::size_t i = 0; i < 6; i++)
   {
      CAPTURE(i);
      CHECK_THAT(psd_interp(sample_freqs[i]), 
                              WithinULP(sample_psd_exact[i], 1));
   }
}

} // namespace jabber_test
