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
      // Interior index to test:
      constexpr std::size_t kIdx = 2;

      constexpr std::size_t N = kFreqSample.size()-1;

      Interval left_exact, interior_exact, right_exact;

      if (method == Interval::Method::Midpoint)
      {
         left_exact = Interval{kFreqSample[0], 
                               (kFreqSample[0]+kFreqSample[1])/2.0};
         interior_exact = Interval{
                              (kFreqSample[kIdx]+kFreqSample[kIdx-1])/2.0, 
                              (kFreqSample[kIdx]+kFreqSample[kIdx+1])/2.0};
         right_exact = Interval{(kFreqSample[N-1]+kFreqSample[N])/2.0, 
                                 kFreqSample[N]};
      }
      else if (method == Interval::Method::MidpointLog10)
      {
         left_exact = Interval{kFreqSample[0], 
                               std::sqrt(kFreqSample[0]*kFreqSample[1])};
         interior_exact = Interval{
                           std::sqrt(kFreqSample[kIdx-1]*kFreqSample[kIdx]),
                           std::sqrt(kFreqSample[kIdx+1]*kFreqSample[kIdx])};
         right_exact = Interval{std::sqrt(kFreqSample[N]*kFreqSample[N-1]), 
                                kFreqSample[N]};                   
      }
      else
      {
         const int m = static_cast<int>(method);
         FAIL("No unit test for interval method enumerator" << m << "!");
      }
      std::map<std::size_t, Interval*> exact_intervals{{0, &left_exact}, 
                                                       {kIdx, &interior_exact},
                                                       {N, &right_exact}};
      for (auto kv : exact_intervals)
      {
         Interval test_interval = 
                     Interval::ComputeInterval(kFreqSample, kv.first, method);
         INFO("Index: " << kv.first);
         CHECK(kv.second->f_left == test_interval.f_left);
         CHECK(kv.second->f_right == test_interval.f_right);
      }
   }

}

TEST_CASE("PWLinearPSD integration", "[PSD]")
{
   SECTION("Single segment")
   {
      //PWLinearPSD psd({kFreqs[0], kFreqs[1]}, {kPsd[0], kPsd[1]});
   }
   // Integrate from 
}

} // namespace jabber_test
