#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <jabber.hpp>

#include <array>
#include <cmath>
#include <functional>

using namespace jabber;
using namespace Catch::Matchers;

namespace jabber_test
{


double ExactLinearIntegral(double x0, double y0, double x1, double y1,
                           double a, double b)
{
   const double m = (y1-y0)/(x1-x0);
   // Definite integral of m(x-x0)+y0 computed via Wolfram Alpha
   return 0.5*(b-a)*(m*(a+b-2*x0)+2*y0);
}


double ExactLogLogIntegral(double x0, double y0, double x1, double y1, 
                           double a, double b)
{
   const double m = std::log10(y1/y0)/std::log10(x1/x0);
   if (m != -1)
   {
      // Definite integral of y0*(x/x0)^m computed via Wolfram Alpha
      return (y0/(m+1))*(b*std::pow(b/x0, m) - a*std::pow(a/x0, m));
   }
   else
   {
      // Definite integral of y0*(x/x0)^(-1) computed via Wolfram Alpha
      return x0*y0*(std::log(b/a));
   }
}

// Verify verification functions above
TEST_CASE("Integration function verification", "[PSD]")
{
   SECTION("Linear integral")
   {
      REQUIRE_THAT(ExactLinearIntegral(1e1,1e-8,1e3,1e-12,1e1,1e3), 
                     WithinULP(4.950495e-6, 0));
   }

   SECTION("Log-log integral")
   {
      SECTION("m != -1")
      {
         REQUIRE_THAT(ExactLogLogIntegral(1e1,1e-8,1e3,1e-12,1e1,1e3),
                      WithinULP(9.9e-8, 0));
      }
      SECTION("m=-1")
      {
         REQUIRE_THAT(ExactLogLogIntegral(1e1, 1e-8, 1e3, 1e-10, 1e1,1e3),
                      WithinULP(1e-7*std::log(100), 0));
      }
   }
}

TEST_CASE("ComputeInterval", "[Interval]")
{

   constexpr std::array<double, 3> kFreqSample={0.1e3, 10e3, 40e3};

   constexpr int kNumIntervalMethods = 
                                 static_cast<int>(Interval::Method::Size);

   // Use generator to ensure that all source types are checked!
   const int m = GENERATE_COPY(range(0, kNumIntervalMethods));
   const Interval::Method method = static_cast<Interval::Method>(m);
   DYNAMIC_SECTION("Interval Method " << m)
   {
      std::array<Interval, 3> exact;
      if (method == Interval::Method::Midpoint)
      {
         exact[0] = Interval{0.1e3, 5.05e3};
         exact[1] = Interval{5.05e3, 25e3};
         exact[2] = Interval{25e3, 40e3};
      }
      else if (method == Interval::Method::MidpointLog10)
      {
         exact[0] = Interval{0.1e3, 1e3};
         exact[1] = Interval{1e3, 20e3};
         exact[2] = Interval{20e3, 40e3};                   
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
         CHECK(exact[i].f_left == test_interval.f_left);
         CHECK(exact[i].f_right == test_interval.f_right);
      }
   }

}

TEST_CASE("PWLinearPSD integration", "[PSD]")
{
   SECTION("Single segment")
   {
      constexpr std::array<double,2> kX{1.0, 3.0}, 
                                     kY{2.0, 4.0};
      constexpr double kM = (kY[1]-kY[0])/(kX[1]-kX[0]);

      const double kA=GENERATE(0.5,1.0,1.5); 
      const double kB=GENERATE(3.5,4.0,4.5);
      
      // Definite integral of m(x-x0)+y0 computed via Wolfram Alpha
      const double kExact = 0.5*(kB-kA)*(kM*(kA+kB-2*kX[0])+2*kY[0]);

      PWLinearPSD psd(kX,kY);
      CAPTURE(kA, kB);
      REQUIRE(psd.Integrate(kA, kB) == kExact);
   }

   SECTION("Multi-segment")
   {
      constexpr std::array<double,4> kX{1.0, 3.0,  5.0,  7.0}, 
                                     kY{2.0, 4.0, -6.0, -8.0};
      constexpr std::size_t kNumIv = kX.size() -1;
      constexpr std::array<double,kNumIv> kM = 
      [&]()
      {
         std::array<double, kNumIv> slopes;
         for (std::size_t i = 0; i < kNumIv; i++)
         {
            slopes[i] = (kY[i+1]-kY[i])/(kX[i+1]-kX[i]);
         }
         return slopes;
      }();

      // Define function pointers of integral for each interval
      const std::array<std::function<double(double,double)>,kNumIv> kExact =
      [&]()
      {
         std::array<std::function<double(double,double)>,kNumIv> exact_funcs;
         for (std::size_t i = 0; i < kNumIv; i++)
         {
            exact_funcs[i] = 
            [&,i](double a, double b) -> double
            {
               return 0.5*(b-a)*(kM[i]*(a+b-2*kX[i])+2*kY[i]);
            };
         }
         return exact_funcs;
      }();

      const double kA=GENERATE(0.5, 1.0, 1.5, 3.0, 3.3); 
      const double kB=GENERATE(3.5, 5.0, 5.5, 7.0, 8.0);

      if (kA > kB)
      {
         FAIL("Error in test configuration. kA cannot be > kB.");
      }

      double exact = 0.0;
      for (std::size_t i = 0; i < kNumIv; i++)
      {
         double left_bound = 0.0;
         double right_bound = 0.0;

         // End if we have finished integration.
         if (kB < kX[i])
         {
            break;
         }
         // If a is before/at interval start AND not ahead of first interval
         if (kA <= kX[i] && i != 0)
         {
            left_bound = kX[i];
         }
         // Else if a is within interval OR ahead of first interval
         else if (kA < kX[i+1])
         {
            left_bound = kA;
         }
         // else not yet at interval of integration start
         else
         {
            continue;
         }

         // If b is after/at interval end AND not behind final interval
         if (kB >= kX[i+1] && i+1 != kNumIv)
         {
            right_bound = kX[i+1];
         }
         // Else must be within interval
         else
         {
            right_bound = kB;
         }
         exact += kExact[i](left_bound, right_bound);
      }
      
      PWLinearPSD psd(kX,kY);
      CAPTURE(kA, kB);
      REQUIRE_THAT(psd.Integrate(kA, kB), WithinULP(exact, 5));
   }
}

TEST_CASE("PWLogLogPSD integration", "[PSD]")
{
   SECTION("Single segment")
   {
      constexpr std::array<double,2> kX{5e3, 20e3}, 
                                     kY{1e-8, 1e-9};
      const double kM = std::log10(kY[1]/kY[0])/std::log10(kX[1]/kX[0]);

      // Ensure that kM != -1
      REQUIRE(kM != -1);

      const double kA=GENERATE(2.5e3,5e3,10e3); 
      const double kB=GENERATE(15e3, 20e3, 25e3);
      
      // Definite integral of y0*(x/x0)^m computed via Wolfram Alpha
      const double kExact = (kY[0]/(kM+1))*(kB*std::pow(kB/kX[0], kM)
                                             -kA*std::pow(kA/kX[0], kM));

      PWLogLogPSD psd(kX,kY);
      CAPTURE(kA, kB);
      CAPTURE(psd.Integrate(kA, kB), kExact);
      REQUIRE_THAT(psd.Integrate(kA, kB), WithinULP(kExact,5));
   }

   SECTION("Single segment, m=-1")
   {
      constexpr std::array<double,2> kX{1e3, 10e3}, 
                                     kY{1e-6, 1e-7};
      const double kM = std::log10(kY[1]/kY[0])/std::log10(kX[1]/kX[0]);

      // Ensure that m == -1
      REQUIRE(kM == -1);

      const double kA=GENERATE(0.5e3,1e3,2.5e3); 
      const double kB=GENERATE(7.5e3, 10e3, 15e3);
      
      // Definite integral of y0*(x/x0)^(-1) computed via Wolfram Alpha
      const double kExact = kX[0]*kY[0]*(std::log(kB)-std::log(kA));

      PWLogLogPSD psd(kX,kY);
      CAPTURE(kA, kB);
      CAPTURE(psd.Integrate(kA, kB), kExact);
      REQUIRE_THAT(psd.Integrate(kA, kB), WithinULP(kExact,5));
   }

   SECTION("Multi-segment")
   {
      constexpr std::array<double,4> kX{1e3,  10e3, 50e3, 95e3}, 
                                     kY{1e-6, 1e-7, 5e-7, 5e-8};
      constexpr std::size_t kNumIv = kX.size() - 1;
      const std::array<double,kNumIv> kM = 
      [&]()
      {
         std::array<double, kNumIv> slopes;
         for (std::size_t i = 0; i < kNumIv; i++)
         {
            slopes[i] = std::log10(kY[i+1]/kY[i])/std::log10(kX[i+1]/kX[i]);
         }
         return slopes;
      }();

      // Define function pointers of integral for each interval
      const std::array<std::function<double(double,double)>,kNumIv> kExact =
      [&]()
      {
         std::array<std::function<double(double,double)>,kNumIv> exact_funcs;
         for (std::size_t i = 0; i < kNumIv; i++)
         {
            exact_funcs[i] = 
            [&,i](double a, double b) -> double
            {
               if (kM[i] == -1)
               {
                  return kX[i]*kY[i]*(std::log(b)-std::log(a));
               }
               else
               {
                  return (kY[i]/(kM[i]+1))*(b*std::pow(b/kX[i], kM[i])
                                             -a*std::pow(a/kX[i], kM[i]));
               }
            };
         }
         return exact_funcs;
      }();

      const double kA=GENERATE(0.5e3, 1e3, 5e3, 10e3, 15e3); 
      const double kB=GENERATE(30e3, 50e3, 75e3, 95e3, 150e3);

      if (kA > kB)
      {
         FAIL("Error in test configuration. kA cannot be > kB.");
      }

      double exact = 0.0;
      for (std::size_t i = 0; i < kNumIv; i++)
      {
         double left_bound = 0.0;
         double right_bound = 0.0;

         // End if we have finished integration.
         if (kB < kX[i])
         {
            break;
         }
         // If a is before/at interval start AND not ahead of first interval
         if (kA <= kX[i] && i != 0)
         {
            left_bound = kX[i];
         }
         // Else if a is within interval OR ahead of first interval
         else if (kA < kX[i+1])
         {
            left_bound = kA;
         }
         // else not yet at interval of integration start
         else
         {
            continue;
         }

         // If b is after/at interval end AND not behind final interval
         if (kB >= kX[i+1] && i+1 != kNumIv)
         {
            right_bound = kX[i+1];
         }
         // Else must be within interval
         else
         {
            right_bound = kB;
         }
         exact += kExact[i](left_bound, right_bound);
      }
      
      PWLogLogPSD psd(kX,kY);
      CAPTURE(kA, kB);
      REQUIRE_THAT(psd.Integrate(kA, kB), WithinULP(exact, 5));
   }
}

// BasePSD::Discretize is quite simple.
// ComputeInterval + Integrate mostly covers its functionality.
// Simple test with just PWLogLogPSD below.
TEST_CASE("PSD Discretization", "[PSD]")
{
   constexpr std::array<double,4> kFreq{1e3,  10e3, 50e3, 95e3}, 
                                    kPsd{1e-6, 1e-7, 5e-7, 5e-8};
   constexpr std::array<double,2> kFreqSample{5e3, 50e3};

   PWLogLogPSD psd(kFreq, kPsd);

   double exact = 0.0;
   //exact += 
}

} // namespace jabber_test
