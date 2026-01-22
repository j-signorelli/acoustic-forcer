#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <jabber.hpp>

#include <cmath>
#include <random>

using namespace jabber;
using namespace Catch::Matchers;

static constexpr std::size_t NUM_PTS = 10;
static constexpr std::size_t NUM_TIMES = 10;
static constexpr int SEED = 0;

/**
 * @brief **Important:** tests values must fall within this
 * many floating-point numbers from the provided actual solution.
 * Simply put, this should be reasonably small. If tests are failing,
 * try increasing this and checking if values being compared are still
 * "equal enough"!
 */
static constexpr std::uint64_t ULP = 5;

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

TEST_CASE("Flowfield kernel computation", "[Kernels]")
{
   SECTION("Single Wave")
   {
      // Base flow:
      constexpr double rho_bar = 0.1792;
      constexpr double p_bar = 2000.0;
      constexpr std::array<double,3> U_bar = {1000.0, 0.0, 0.0};
      constexpr double gamma = 1.4;
      const double c_bar = std::sqrt(gamma*p_bar/rho_bar);
      
      // Wave params:
      constexpr double p_amp = 10.0;
      constexpr double freq = 1000.0;
      constexpr double phase = M_PI/3;
      
      SECTION("1D")
      {  
         // Coords + times to initialize
         std::array<double, NUM_PTS> coords;
         std::array<double, NUM_TIMES> times;

         // Kernel variables
         constexpr double omega = 2*M_PI*freq;
         constexpr double mod_wave_dir = -1.0;
         const double k = omega/(U_bar[0] - c_bar);
         
         std::array<double, NUM_PTS> k_dot_phi_p_phi;
         std::array<double, NUM_PTS> rho;
         std::array<double, NUM_PTS> rhoV;
         std::array<double, NUM_PTS> rhoE;

         std::mt19937 gen(SEED);
         
         // c/f = 0.125 -- include multiple spatial periods...
         std::uniform_real_distribution<double> real_dist_x(0.0, 1.0);

         // 1/f = 0.001 -- include multiple temporal periods...
         std::uniform_real_distribution<double> real_dist_t(0.0, 0.005);

         for (std::size_t i = 0; i < NUM_PTS; i++)
         {
            coords[i] = real_dist_x(gen);
            k_dot_phi_p_phi[i] = k*coords[i] + phase;
         }
         for (std::size_t i = 0; i < NUM_TIMES; i++)
         {
            times[i] = real_dist_t(gen);
         }

         // Evaluate kernel
         for (const double &time : times)
         {
            // Compute
            ComputeKernel<1>(NUM_PTS, rho_bar, p_bar, U_bar.data(), gamma, 1,
                              &p_amp, &omega, &mod_wave_dir, 
                              k_dot_phi_p_phi.data(), time, rho.data(), 
                              rhoV.data(), rhoE.data());

            // Check solutions
            for (std::size_t i = 0; i < NUM_PTS; i++)
            {
               CAPTURE(coords[i], time);
               CHECK_THAT(rho[i], 
                        WithinULP(SingleWave1DSolution::Rho(coords[i],
                                    time), ULP));
               CHECK_THAT(rhoV[i], 
                        WithinULP(SingleWave1DSolution::RhoU(coords[i], 
                                    time), ULP));
               CHECK_THAT(rhoE[i], 
                        WithinULP(SingleWave1DSolution::RhoE(coords[i],
                                    time), ULP));
            }
         }
         
      }

      SECTION("2D")
      {

      }

      SECTION("3D")
      {

      }
   }
}

