#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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
static constexpr double p_amp = 10.0;
static constexpr double freq = 1000.0;
static constexpr double phase = M_PI/3;

// Coordinates data | c/f = 0.125 -- include multiple spatial periods...
static const std::array<double, NUM_PTS> coords 
                        = GenerateRandomArr<NUM_PTS>(SEED,0.0,1.0);

// Time data | 1/f = 0.001 -- include multiple temporal periods...
static const std::array<double, NUM_TIMES> times 
                        = GenerateRandomArr<NUM_TIMES>(SEED,0.0,0.005);

/// Hardcoded analytical solution. See README.md.
struct SingleWave1DSolution
{
private:
   static double PPrime(double x, double t)
   {
      return 10.0*std::cos((16.0*M_PI/7.0)*x + M_PI/3.0 - 2000.0*M_PI*t);
   }
   static double U(double x, double t)
   {
      return (1000.0 + (-1.0/(0.1792*125.0))*PPrime(x,t));
   }
public:
   static double Rho(double x, double t)
   {
      return 0.1792 + (1.0/15625.0)*PPrime(x,t);
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
                              double t)
{
   // Check solutions
   for (std::size_t i = 0; i < x.size(); i++)
   {
      CAPTURE(x[i], t);
      CHECK_THAT(rho[i], WithinULP(SingleWave1DSolution::Rho(x[i], t), ULP));
      CHECK_THAT(rhoU[i], WithinULP(SingleWave1DSolution::RhoU(x[i], t), ULP));
      CHECK_THAT(rhoE[i], WithinULP(SingleWave1DSolution::RhoE(x[i],t), ULP));
   }
}

TEST_CASE("Single 1D wave computation via kernel", "[Kernels]")
{
   // Initialize kernel variables
   constexpr double omega = 2*M_PI*freq;
   constexpr double mod_wave_dir = -1.0;
   const double c_bar = std::sqrt(gamma*p_bar/rho_bar);
   const double k = omega/(U_bar - c_bar);
   
   std::array<double, NUM_PTS> k_dot_x_p_phi;
   for (std::size_t i = 0; i < NUM_PTS; i++)
   {
      k_dot_x_p_phi[i] = k*coords[i] + phase;
   }

   std::array<double, NUM_PTS> rho;
   std::array<double, NUM_PTS> rhoU;
   std::array<double, NUM_PTS> rhoE;


   // Evaluate kernel
   for (const double &time : times)
   {
      // Compute
      ComputeKernel<1>(NUM_PTS, rho_bar, p_bar, &U_bar, gamma, 1,
                        &p_amp, &omega, &mod_wave_dir, 
                        k_dot_x_p_phi.data(), time, rho.data(), 
                        rhoU.data(), rhoE.data());

      // Check solutions
      CheckSolution(coords, rho, rhoU, rhoE, time);
   }
}

TEST_CASE("Single 1D wave computation via AcousticField", 
            "[AcousticField]")
{
   // Build AcousticField
   std::vector<double> U_bar_vec = {U_bar};
   AcousticField field(1, coords, p_bar, rho_bar, U_bar_vec, gamma);

   // Add wave + finalize
   std::vector<double> dir_vec = {1.0};
   Wave single_wave{p_amp, freq, phase, true, dir_vec};
   field.AddWave(single_wave);
   field.Finalize();

   // Evaluate field
   for (const double &time : times)
   {
      // Compute
      field.Compute(time);

      // Check solutions
      CheckSolution(coords, field.Density(), field.Momentum(),
                     field.Energy(), time);
   }

}

#ifdef JABBER_WITH_APP

TEST_CASE("Single 1D wave computation via app library", "[App]")
{
   ConfigInput config;

   // Set base flow in config
   BaseFlowParams &base_flow = config.BaseFlow();
   base_flow.rho = rho_bar;
   base_flow.p = p_bar;
   base_flow.U = std::vector<double>(1, U_bar);
   base_flow.gamma = gamma;

   // Set source in config
   SourceParams<SourceOption::SingleWave> wave;
   wave.amp = p_amp;
   wave.direction.resize(1, 1.0);
   wave.freq = freq;
   wave.phase = phase*180.0/M_PI; // in degrees!
   wave.speed = SpeedOption::Slow;

   // Add wave to Config sources
   config.Sources().push_back(wave);

   // Initialize AcousticField
   AcousticField field = InitializeAcousticField(config, coords, 1);

   // Evaluate field
   for (const double &time : times)
   {
      // Compute
      field.Compute(time);

      // Check solutions
      CheckSolution(coords, field.Density(), field.Momentum(),
                     field.Energy(), time);
   }
}

#endif // JABBER_WITH_APP

} // jabber_test
