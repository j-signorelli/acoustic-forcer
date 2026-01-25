#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <jabber.hpp>
#ifdef JABBER_WITH_APP
#include <jabber_app.hpp>
#endif // JABBER_WITH_APP

#include <cmath>

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
static constexpr std::uint64_t ULP = 5;

/// Number of points to initialize + test at
static constexpr std::size_t NUM_PTS = 10;

/// Number of times to test at
static constexpr std::size_t NUM_TIMES = 10;

/// Seed for randomizer
static constexpr int SEED = 0;

/// Base flow params:
static constexpr double rho_bar = 0.1792;
static constexpr double p_bar = 2000.0;
static constexpr double U_bar = 1000.0;
static constexpr double gamma = 1.4;

/// Wave params:
static constexpr std::array<double, 2> p_amps = {10.0, 5.0};
static constexpr std::array<double, 2> freqs = {1000.0, 1250.0};
static constexpr std::array<double, 2> phases = {M_PI/3, M_PI};
static constexpr std::array<bool, 2> speeds = {true, false};
/**
 * @brief Coordinates data.
 * 
 * @details Wave 1 spatial period = 2\pi/k_1=7/8. Wave 2 spatial
 * period = 2\pi/k_2=9/10. Thus, choose 2.0 to cover ~2 periods.
 * 
 * 
 */
static const std::array<double, NUM_PTS> coords 
                        = GenerateRandomArr<NUM_PTS>(SEED,0.0,2.0);

/**
 * @brief Time data.
 * 
 * @details Wave 1 temporal period = 1/f_1=0.001. Wave 2 temporal
 * period = 1/f_2=0.0008. Choose 0.002 to cover ~2 periods.
 * 
 */
static const std::array<double, NUM_TIMES> times 
                        = GenerateRandomArr<NUM_TIMES>(SEED,0.0,0.002);

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

static void CheckSolution(std::span<const double> x, 
                              std::span<const double> rho,
                              std::span<const double> rhoU, 
                              std::span<const double> rhoE,
                              double t, int num_waves)
{
   // Check solutions
   if (num_waves == 1)
   {
      for (std::size_t i = 0; i < x.size(); i++)
      {
         CAPTURE(x[i], t);
         CHECK_THAT(rho[i], 
                     WithinULP(AnalyticalSolution1D<1>::Rho(x[i], t), ULP));
         CHECK_THAT(rhoU[i], 
                     WithinULP(AnalyticalSolution1D<1>::RhoU(x[i], t), ULP));
         CHECK_THAT(rhoE[i], 
                     WithinULP(AnalyticalSolution1D<1>::RhoE(x[i],t), ULP));
      }
   }
   else if (num_waves == 2)
   {
      for (std::size_t i = 0; i < x.size(); i++)
      {
         CAPTURE(x[i], t);
         CHECK_THAT(rho[i], 
                     WithinULP(AnalyticalSolution1D<2>::Rho(x[i], t), ULP));
         CHECK_THAT(rhoU[i], 
                     WithinULP(AnalyticalSolution1D<2>::RhoU(x[i], t), ULP));
         CHECK_THAT(rhoE[i], 
                     WithinULP(AnalyticalSolution1D<2>::RhoE(x[i],t), ULP));
      }
   }
   else
   {
      FAIL("Test does not support number of waves = " << num_waves);
   }
}

TEST_CASE("Flowfield computation via kernel", "[Compute] [Kernels]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      const double c_bar = std::sqrt(gamma*p_bar/rho_bar);

      std::vector<double> k_dot_x_p_phi(NUM_PTS*kNumWaves);
      std::vector<double> omega(kNumWaves);
      std::vector<double> mod_wave_dir(kNumWaves);

      // Initialize kernel variables
      for (int w = 0; w < kNumWaves; w++)
      {
         omega[w] = 2*M_PI*freqs[w];
         mod_wave_dir[w] = (speeds[w] ? -1.0 : 1.0);

         const double k = speeds[w] ? omega[w]/(U_bar - c_bar) 
                                    : omega[w]/(U_bar + c_bar);
         
         for (std::size_t i = 0; i < NUM_PTS; i++)
         {
            k_dot_x_p_phi[w*NUM_PTS + i] = k*coords[i] + phases[w];
         }
      }

      std::array<double, NUM_PTS> rho;
      std::array<double, NUM_PTS> rhoU;
      std::array<double, NUM_PTS> rhoE;

      // Evaluate kernel
      for (const double &time : times)
      {
         // Compute
         ComputeKernel<1>(NUM_PTS, rho_bar, p_bar, &U_bar, gamma, kNumWaves,
                           p_amps.data(), omega.data(), mod_wave_dir.data(), 
                           k_dot_x_p_phi.data(), time, rho.data(), 
                           rhoU.data(), rhoE.data());

         // Check solutions
         CheckSolution(coords, rho, rhoU, rhoE, time, kNumWaves);
      }
   }
}

TEST_CASE("Flowfield computation via AcousticField", 
            "[Compute] [AcousticField]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      // Build AcousticField
      std::vector<double> U_bar_vec = {U_bar};
      AcousticField field(1, coords, p_bar, rho_bar, U_bar_vec, gamma);

      // Add wave(s) + finalize
      std::vector<double> dir_vec = {1.0};
      for (int w = 0; w < kNumWaves; w++)
      {
         Wave wave{p_amps[w], freqs[w], phases[w], speeds[w], dir_vec};
         field.AddWave(wave);
      }
      field.Finalize();

      // Evaluate field
      for (const double &time : times)
      {
         // Compute
         field.Compute(time);

         // Check solutions
         CheckSolution(coords, field.Density(), field.Momentum(),
                        field.Energy(), time, kNumWaves);
      }
   }
}

#ifdef JABBER_WITH_APP

TEST_CASE("Single 1D wave computation via app library", "[Compute] [App]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      ConfigInput config;

      // Set base flow in config
      BaseFlowParams &base_flow = config.BaseFlow();
      base_flow.rho = rho_bar;
      base_flow.p = p_bar;
      base_flow.U = std::vector<double>(1, U_bar);
      base_flow.gamma = gamma;

      // Set source in config
      for (int w = 0; w < kNumWaves; w++)
      {
         SourceParams<SourceOption::SingleWave> wave;
         wave.amp = p_amps[w];
         wave.direction.resize(1, 1.0);
         wave.freq = freqs[w];
         wave.phase = phases[w]*180.0/M_PI; // in degrees!
         wave.speed = speeds[w] ? SpeedOption::Slow : SpeedOption::Fast;

         // Add wave to Config sources
         config.Sources().push_back(wave);
      }

      // Initialize AcousticField
      AcousticField field = InitializeAcousticField(config, coords, 1);

      // Evaluate field
      for (const double &time : times)
      {
         // Compute
         field.Compute(time);

         // Check solutions
         CheckSolution(coords, field.Density(), field.Momentum(),
                        field.Energy(), time, kNumWaves);
      }
   }
}

#endif // JABBER_WITH_APP

} // jabber_test
