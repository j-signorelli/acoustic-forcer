#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_template_test_macros.hpp>

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
static const std::vector<double> kUBar = {600.0, 800.0};
static constexpr double kGamma = 1.4;

/// Wave params:
static constexpr std::array<double, 2> kPAmps = {10.0, 5.0};
static constexpr std::array<double, 2> kFreqs = {1000.0, 1250.0};
static constexpr std::array<double, 2> kPhases = {M_PI/3, M_PI};
static constexpr std::array<char, 2> kSpeeds = {'S', 'F'};
static const std::array<std::vector<double>,2> 
                     kWaveDirs = {std::vector<double>({1.0, 0.0}),
                                    std::vector<double>({6.0/10.0, 8.0/10.0})};
/**
 * @brief Coordinates data, in XY XY ordering.
 * 
 * @details Same extent as 1D, in both directions.
 */
static const std::array<double, kNumPts*2> kCoords
                        = GenerateRandomArr<kNumPts*2>(kSeed,0.0,2.0);

/**
 * @brief Time data.
 * 
 * @details Same time range as 1D.
 */
static const std::array<double, kNumTimes> kTimes 
                        = GenerateRandomArr<kNumTimes>(kSeed,0.0,0.002);

/// Hardcoded analytical solution. See README.md.
template<int NumWaves>
   requires (NumWaves == 1 || NumWaves == 2)
struct AnalyticalSolution2D
{
private:
   static double PPrimeWave1(double x, double y, double t)
   {
      return 10.0*std::cos((80.0*M_PI/19.0)*x + M_PI/3.0 
                                                      - 2000.0*M_PI*t);
   }
   static double PPrimeWave2(double x, double y, double t)
   {
      return 5.0*std::cos((4.0*M_PI/3.0)*x + (16.0*M_PI/9.0)*y + M_PI
                                                      - 2500.0*M_PI*t);
   }
   static double PPrime(double x, double y, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return PPrimeWave1(x,y,t);
      }
      else
      {
         return PPrimeWave1(x,y,t) + PPrimeWave2(x,y,t);
      }
   }
   static double Ux(double x, double y, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 600.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,y,t));
      }
      else
      {
         return 600.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,y,t)
                                             + (6.0/10.0)*PPrimeWave2(x,y,t));
      }
   }
   static double Uy(double x, double y, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 800.0;
      }
      else
      {
         return 800.0 + (1.0/(0.1792*125.0))*((8.0/10.0)*PPrimeWave2(x,y,t));
      }
   }
public:
   static double Rho(double x, double y, double t)
   {
      return 0.1792 + (1.0/15625.0)*(PPrime(x,y,t));
   }
   static double RhoUx(double x, double y, double t)
   {
      return Rho(x,y,t)*Ux(x,y,t);
   }
   static double RhoUy(double x, double y, double t)
   {
      return Rho(x,y,t)*Uy(x,y,t);
   }
   static double RhoE(double x, double y, double t)
   {
      return (2000.0/(1.4-1.0)) + (1.0/(1.4-1.0))*PPrime(x,y,t)
               + (1.0/2.0)*Rho(x,y,t)*(Ux(x,y,t)*Ux(x,y,t) 
                                       + Uy(x,y,t)*Uy(x,y,t));
   }
};

/**
 * @brief Check solution against analytical. Note that \p coords is in
 * XY XY ordering, while \p rhoU is in XX YY ordering.
 */
static void CheckSolution(std::span<const double> coords,
                           std::span<const double> rho,
                           std::span<const double> rhoU,
                           std::span<const double> rhoE,
                           double t, int num_waves)
{
   using function_t = std::function<double(double,double,double)>;
   function_t rho_exact, rhoUx_exact, rhoUy_exact, rhoE_exact;

   if (num_waves == 1)
   {
      rho_exact = AnalyticalSolution2D<1>::Rho;
      rhoUx_exact = AnalyticalSolution2D<1>::RhoUx;
      rhoUy_exact = AnalyticalSolution2D<1>::RhoUy;
      rhoE_exact = AnalyticalSolution2D<1>::RhoE;
   }
   else if (num_waves == 2)
   {
      rho_exact = AnalyticalSolution2D<2>::Rho;
      rhoUx_exact = AnalyticalSolution2D<2>::RhoUx;
      rhoUy_exact = AnalyticalSolution2D<2>::RhoUy;
      rhoE_exact = AnalyticalSolution2D<2>::RhoE;
   }
   else
   {
      FAIL("Test does not support number of waves = " << num_waves);
   }

   for (std::size_t i = 0; i < kNumPts; i++)
   {
      const double x = coords[i*2];
      const double y = coords[i*2+1];
      CAPTURE(x,y,t);
      CHECK_THAT(rho[i], WithinULP(rho_exact(x,y,t), kULP));
      CHECK_THAT(rhoU[i], WithinULP(rhoUx_exact(x,y,t), kULP));
      CHECK_THAT(rhoU[kNumPts+i], WithinULP(rhoUy_exact(x,y,t), kULP));
      CHECK_THAT(rhoE[i], WithinULP(rhoE_exact(x,y,t), kULP));
   }
}

TEMPLATE_TEST_CASE_SIG("2D flowfield computation via kernel",
                       "[2D][Compute][Kernels]",
                        ((bool TGridInnerLoop), TGridInnerLoop),
                        true, false)
{
   CAPTURE(TGridInnerLoop);

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
      std::vector<double> mod_wave_dir(kNumWaves*2);

      // Initialize kernel variables
      for (int w = 0; w < kNumWaves; w++)
      {
         omega[w] = 2*M_PI*kFreqs[w];

         const std::vector<double> k_hat = kWaveDirs[w];
         const double mod_fac = (kSpeeds[w] == 'S' ? -1.0 : 1.0);

         mod_wave_dir[w] = mod_fac*k_hat[0];
         mod_wave_dir[kNumWaves+w] = mod_fac*k_hat[1];
         const double U_bar_dot_k_hat = kUBar[0]*k_hat[0] + kUBar[1]*k_hat[1];

         const double k = (kSpeeds[w] == 'S' 
                                    ? omega[w]/(U_bar_dot_k_hat - c_bar)
                                    : omega[w]/(U_bar_dot_k_hat + c_bar));
         
         for (std::size_t i = 0; i < kNumPts; i++)
         {
            const std::size_t idx = TGridInnerLoop ? w*kNumPts + i 
                                                   : i*kNumWaves + w;

            k_dot_x_p_phi[idx] = k*(k_hat[0]*kCoords[i*2] 
                                       + k_hat[1]*kCoords[i*2+1])
                                    + kPhases[w];
         }
      }

      std::array<double, kNumPts> rho;
      std::array<double, kNumPts*2> rhoU;
      std::array<double, kNumPts> rhoE;

      // Evaluate kernel
      for (const double &time : kTimes)
      {
         // Compute
         ComputeKernel<2, TGridInnerLoop>(kNumPts, kRhoBar, kPBar, 
                           kUBar.data(), kGamma, kNumWaves, kPAmps.data(),
                           omega.data(), mod_wave_dir.data(), 
                           k_dot_x_p_phi.data(), time, 
                           rho.data(), rhoU.data(), rhoE.data());

         // Check solutions
         CheckSolution(kCoords, rho, rhoU, rhoE, time, kNumWaves);
      }
   }
}

TEST_CASE("2D flowfield computation via AcousticField", 
            "[2D][Compute][AcousticField]")
{
   const AcousticField::Kernel kernel = 
                        GENERATE(options<AcousticField::Kernel>());
   CAPTURE(kernel);

   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      // Build AcousticField
      AcousticField field(2, kCoords, kPBar, kRhoBar, kUBar, kGamma, kernel);

      // Add wave(s) + finalize
      for (int w = 0; w < kNumWaves; w++)
      {
         Wave wave{kPAmps[w], kFreqs[w], kPhases[w], kSpeeds[w], kWaveDirs[w]};
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

TEST_CASE("2D flowfield computation via app library", "[2D][Compute][App]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      ConfigInput config;

      // Set base flow in config
      BaseFlowParams &base_flow = config.BaseFlow();
      base_flow.rho = kRhoBar;
      base_flow.p = kPBar;
      base_flow.U = kUBar;
      base_flow.gamma = kGamma;

      // Set source in config
      for (int w = 0; w < kNumWaves; w++)
      {
         SourceParams<SourceOption::SingleWave> wave;
         wave.amp = kPAmps[w];
         wave.direction = kWaveDirs[w];
         wave.freq = kFreqs[w];
         wave.phase = kPhases[w]*180.0/M_PI; // in degrees!
         wave.speed = kSpeeds[w];

         // Add wave to Config sources
         config.Sources().push_back(wave);
      }

      // Set kernel
      config.Comp().kernel = GENERATE(options<AcousticField::Kernel>());

      // Initialize AcousticField
      AcousticField field = InitializeAcousticField(config, kCoords, 2);

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
