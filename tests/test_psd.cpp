#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <jabber.hpp>

#include <array>
#include <cmath>

using namespace jabber;
using namespace Catch::Matchers;

namespace jabber_test
{
// Discrete x for interpolation.
static constexpr std::array<double, 4> kFreqs={0.1e3, 1e3, 10e3, 100e3};

// Discrete PSDs for interpolation.
static constexpr std::array<double, 4> kPsd={1e-8, 5e-8, 7e-9, 2e-9};

// Frequencies to sample/compute with from interpolated PSD.
static constexpr std::array<double, 5> kFreqSample={0.1e3, 0.7e3, 5e3, 10e3,
                                                      100e3};

TEST_CASE("ComputeInterval", "[PSD]")
{

   constexpr int kNumIntervalMethods = 
                                 static_cast<int>(Interval::Method::Size);

   // Use generator to ensure that all source types are checked!
   const int m = GENERATE_COPY(range(0, kNumIntervalMethods));
   const Interval::Method method = static_cast<Interval::Method>(m);
   DYNAMIC_SECTION("Interval Method " << m)
   {
      std::array<Interval, 5> exact_sample_intervals;
      if (method == Interval::Method::Midpoint)
      {
         exact_sample_intervals = std::array<Interval,5>{
                        Interval{0.1e3, (0.1e3+0.7e3)/2.0},
                        Interval{(0.1e3+0.7e3)/2.0, (0.7e3+5e3)/2.0},
                        Interval{(0.7e3+5e3)/2.0, (5e3+10e3)/2.0},
                        Interval{(5e3+10e3)/2.0, (10e3+100e3)/2.0},
                        Interval{(10e3+100e3)/2.0, 100e3}};
      }
      else if (method == Interval::Method::MidpointLog10)
      {
         exact_sample_intervals = std::array<Interval,5>{
               Interval{0.1e3,std::sqrt(0.1e3*0.7e3)},
               Interval{std::sqrt(0.1e3*0.7e3), std::sqrt(0.7e3*5e3)},
               Interval{std::sqrt(0.7e3*5e3), std::sqrt(5e3*10e3)},
               Interval{std::sqrt(5e3*10e3), std::sqrt(10e3*100e3)},
               Interval{std::sqrt(10e3*100e3), 100e3}};
      }
      else
      {
         const int m = static_cast<int>(method);
         FAIL("No unit test for interval method enumerator" << m << "!");
      }

      for (std::size_t i = 0; i < kFreqSample.size(); i++)
      {
         Interval test_interval = 
                     Interval::ComputeInterval(kFreqSample, i, method);
         CAPTURE(i);
         CHECK(exact_sample_intervals[i].f_left == test_interval.f_left);
         CHECK(exact_sample_intervals[i].f_right == test_interval.f_right);
      }
   }

}

} // namespace jabber_test
