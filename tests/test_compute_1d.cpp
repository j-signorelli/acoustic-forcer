#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <jabber.hpp>
#ifdef JABBER_WITH_APP
#include <jabber_app.hpp>
#endif // JABBER_WITH_APP

#ifdef JABBER_WITH_OPENMP
#include <omp.h>
#endif // JABBER_WITH_OPENMP

#include <cmath>
#include <functional>

using namespace jabber;
using namespace Catch::Matchers;
#ifdef JABBER_WITH_APP
using namespace jabber_app;
#endif // JABBER_WITH_APP

namespace jabber_test
{

/**
 * @details **Important:** tests values must fall within
 * "this-many" floating-point numbers from the provided actual solution.
 * Simply put, this should be reasonably small. If tests are failing,
 * try increasing this and checking if values being compared are still
 * "equal enough"!
 */
static constexpr std::uint64_t kULP = 5;

/// Number of points to initialize + test at
static constexpr std::size_t kNumPts = 5;

/// Number of kTimes to test at
static constexpr std::size_t kNumTimes = 5;

/// Seed for randomizer
static constexpr int kSeed = 0;

/// Base flow params:
static constexpr double kRhoBar = 0.1792;
static constexpr double kPBar = 2000.0;
static constexpr double kUBar = 1000.0;
static constexpr double kGamma = 1.4;

/// Wave params:
static constexpr std::array<double, 2> kPAmps = {10.0, 5.0};
static constexpr std::array<double, 2> kFreqs = {1000.0, 1250.0};
static constexpr std::array<double, 2> kPhases = {M_PI/3, M_PI};
static constexpr std::array<char, 2> kSpeeds = {'S', 'F'};

/**
 * @brief Coordinates data.
 * 
 * @details Wave 1 spatial period = 2\pi/k_1=7/8. Wave 2 spatial
 * period = 2\pi/k_2=9/10. Thus, choose 2.0 to cover ~2 periods.
 * 
 * 
 */
static const std::array<double, kNumPts> kCoords 
                        = GenerateRandomArr<kNumPts>(kSeed,0.0,2.0);

/**
 * @brief Time data.
 * 
 * @details Wave 1 temporal period = 1/f_1=0.001. Wave 2 temporal
 * period = 1/f_2=0.0008. Choose 0.002 to cover ~2 periods.
 * 
 */
static const std::array<double, kNumTimes> kTimes 
                        = GenerateRandomArr<kNumTimes>(kSeed,0.0,0.002);

/// Hardcoded analytical solution. See README.md.
template<int NumWaves>
   requires (NumWaves == 1 || NumWaves == 2)
struct AnalyticalSolution1D
{
private:
   static double PPrimeWave1(double x, double t)
   {
      return 10.0*std::cos((16.0*M_PI/7.0)*x + M_PI/3.0 - 2000.0*M_PI*t);
   }
   static double PPrimeWave2(double x, double t)
   {
      return 5.0*std::cos((20.0*M_PI/9.0)*x + M_PI - 2500.0*M_PI*t);
   }
   static double PPrime(double x, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return PPrimeWave1(x,t);
      }
      else
      {
         return PPrimeWave1(x,t) + PPrimeWave2(x,t);
      }
   }
   static double U(double x, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 1000.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,t));
      }
      else
      {
         return 1000.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,t)
                                                   + PPrimeWave2(x,t));
      }
   }
public:
   static double Rho(double x, double t)
   {
      return 0.1792 + (1.0/15625.0)*(PPrime(x,t));
   }
   static double RhoU(double x, double t)
   {
      return Rho(x,t)*U(x,t);
   }
   static double RhoE(double x, double t)
   {
      return (2000.0/(1.4-1.0)) + (1.0/(1.4-1.0))*PPrime(x,t)
               + (1.0/2.0)*Rho(x,t)*U(x,t)*U(x,t);
   }
};

static void CheckSolution(std::span<const double> coords, 
                              std::span<const double> rho,
                              std::span<const double> rhoU, 
                              std::span<const double> rhoE,
                              double t, int num_waves)
{
   using function_t = std::function<double(double,double)>;
   function_t rho_exact, rhoU_exact, rhoE_exact;

   if (num_waves == 1)
   {
      rho_exact = AnalyticalSolution1D<1>::Rho;
      rhoU_exact = AnalyticalSolution1D<1>::RhoU;
      rhoE_exact = AnalyticalSolution1D<1>::RhoE;
   }
   else if (num_waves == 2)
   {
      rho_exact = AnalyticalSolution1D<2>::Rho;
      rhoU_exact = AnalyticalSolution1D<2>::RhoU;
      rhoE_exact = AnalyticalSolution1D<2>::RhoE;
   }
   else
   {
      FAIL("Test does not support number of waves = " << num_waves);
   }

   for (std::size_t i = 0; i < kNumPts; i++)
   {
      const double x = coords[i];
      CAPTURE(x, t);
      CHECK_THAT(rho[i], WithinULP(rho_exact(x,t), kULP));
      CHECK_THAT(rhoU[i], WithinULP(rhoU_exact(x,t), kULP));
      CHECK_THAT(rhoE[i], WithinULP(rhoE_exact(x,t), kULP));
   }
}

TEST_CASE("1D flowfield computation via kernel", "[1D][Compute][Kernels]")
{

#ifdef JABBER_WITH_OPENMP
   omp_set_dynamic(0);
   omp_set_num_threads(GENERATE(1,2));
#endif // JABBER_WITH_OPENMP

   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      const double c_bar = std::sqrt(kGamma*kPBar/kRhoBar);

      std::vector<double> k_dot_x_p_phi(kNumPts*kNumWaves);
      std::vector<double> omega(kNumWaves);
      std::vector<double> mod_wave_dir(kNumWaves);

      // Initialize kernel variables
      for (int w = 0; w < kNumWaves; w++)
      {
         omega[w] = 2*M_PI*kFreqs[w];
         mod_wave_dir[w] = (kSpeeds[w] == 'S' ? -1.0 : 1.0);

         const double k = (kSpeeds[w] == 'S' ? omega[w]/(kUBar - c_bar)
                                             : omega[w]/(kUBar + c_bar));
         
         for (std::size_t i = 0; i < kNumPts; i++)
         {
            k_dot_x_p_phi[w*kNumPts + i] = k*kCoords[i] + kPhases[w];
         }
      }

      std::array<double, kNumPts> rho;
      std::array<double, kNumPts> rhoU;
      std::array<double, kNumPts> rhoE;

      // Evaluate kernel
      for (const double &time : kTimes)
      {
         // Compute
         ComputeKernel<1>(kNumPts, kRhoBar, kPBar, &kUBar, kGamma, kNumWaves,
                           kPAmps.data(), omega.data(), mod_wave_dir.data(), 
                           k_dot_x_p_phi.data(), time, rho.data(), 
                           rhoU.data(), rhoE.data());

         // Check solutions
         CheckSolution(kCoords, rho, rhoU, rhoE, time, kNumWaves);
      }
   }
}

TEST_CASE("1D flowfield computation via AcousticField", 
            "[1D][Compute][AcousticField]")
{
#ifdef JABBER_WITH_OPENMP
   omp_set_dynamic(0);
   omp_set_num_threads(GENERATE(1,2));
#endif // JABBER_WITH_OPENMP

   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      // Build AcousticField
      std::vector<double> kUBar_vec = {kUBar};
      AcousticField field(1, kCoords, kPBar, kRhoBar, kUBar_vec, kGamma);

      // Add wave(s) + finalize
      std::vector<double> dir_vec = {1.0};
      for (int w = 0; w < kNumWaves; w++)
      {
         Wave wave{kPAmps[w], kFreqs[w], kPhases[w], kSpeeds[w], dir_vec};
         field.AddWave(wave);
      }
      field.Finalize();

      // Evaluate field
      for (const double &time : kTimes)
      {
         // Compute
         field.Compute(time);

         // Check solutions
         CheckSolution(kCoords, field.Density(), field.Momentum(),
                        field.Energy(), time, kNumWaves);
      }
   }
}

#ifdef JABBER_WITH_APP

TEST_CASE("1D flowfield computation via app library", "[1D][Compute][App]")
{
#ifdef JABBER_WITH_OPENMP
   omp_set_dynamic(0);
   omp_set_num_threads(GENERATE(1,2));
#endif // JABBER_WITH_OPENMP

   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      ConfigInput config;

      // Set base flow in config
      BaseFlowParams &base_flow = config.BaseFlow();
      base_flow.rho = kRhoBar;
      base_flow.p = kPBar;
      base_flow.U = std::vector<double>(1, kUBar);
      base_flow.gamma = kGamma;

      // Set source in config
      for (int w = 0; w < kNumWaves; w++)
      {
         SourceParams<SourceOption::SingleWave> wave;
         wave.amp = kPAmps[w];
         wave.direction.resize(1, 1.0);
         wave.freq = kFreqs[w];
         wave.phase = kPhases[w]*180.0/M_PI; // in degrees!
         wave.speed = kSpeeds[w];

         // Add wave to Config sources
         config.Sources().push_back(wave);
      }

      // Initialize AcousticField
      AcousticField field = InitializeAcousticField(config, kCoords, 1);

      // Evaluate field
      for (const double &time : kTimes)
      {
         // Compute
         field.Compute(time);

         // Check solutions
         CheckSolution(kCoords, field.Density(), field.Momentum(),
                        field.Energy(), time, kNumWaves);
      }
   }
}

#endif // JABBER_WITH_APP

} // jabber_test
