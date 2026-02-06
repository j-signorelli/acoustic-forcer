#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <jabber.hpp>

#include <cmath>

using namespace jabber;

namespace jabber_test
{

// Discrete x for interpolation.
static constexpr std::array<double, 4> kX={0.1e3, 1e3, 10e3, 100e3};

// Discrete y for interpolation.
static constexpr std::array<double, 4> kY={1e-8, 5e-8, 7e-9, 2e-9};

// Samples to evaluate at.
static constexpr std::array<double, 6> kXSample={0.05e3, 0.7e3, 5e3, 10e3,
                                                 50e3, 150e3};

TEST_CASE("PWLinear", "[Interpolant]")
{
   PWLinear interp(kX, kY);
   constexpr std::array<double, kXSample.size()> kYSample=
      {(5e-8-1e-8)/(1e3-0.1e3)*(0.05e3-0.1e3) + 1e-8,
       (5e-8-1e-8)/(1e3-0.1e3)*(0.7e3-0.1e3) + 1e-8,
       (7e-9-5e-8)/(10e3-1e3)*(5e3-1e3) + 5e-8,
       7e-9,
       (2e-9-7e-9)/(100e3-10e3)*(50e3-10e3) + 7e-9,
       (2e-9-7e-9)/(100e3-10e3)*(150e3-10e3) + 7e-9};
   
   std::array<double, kXSample.size()> y_sample_interp;
   for (std::size_t i = 0; i < kXSample.size(); i++)
   {
      CAPTURE(i);
      CHECK(interp(kXSample[i]) == kYSample[i]);
   }
}

TEST_CASE("PWLogLog", "[Interpolant]")
{
   PWLogLog interp(kX, kY);
   const std::array<double, kXSample.size()> kYSample=
      {1e-8*std::pow((0.05e3/0.1e3), std::log10(5e-8/1e-8)/std::log10(1e3/0.1e3)),
       1e-8*std::pow((0.7e3/0.1e3), std::log10(5e-8/1e-8)/std::log10(1e3/0.1e3)),
       5e-8*std::pow((5e3/1e3), std::log10(7e-9/5e-8)/std::log10(10e3/1e3)),
       7e-9,
       7e-9*std::pow((50e3/10e3), std::log10(2e-9/7e-9)/std::log10(100e3/10e3)),
       7e-9*std::pow((150e3/10e3), std::log10(2e-9/7e-9)/std::log10(100e3/10e3))};
   
   std::array<double, kXSample.size()> y_sample_interp;
   for (std::size_t i = 0; i < kXSample.size(); i++)
   {
      CAPTURE(i);
      CHECK(interp(kXSample[i]) == kYSample[i]);
   }
}

} // namespace jabber_test
