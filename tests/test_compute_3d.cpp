#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <jabber.hpp>
#ifdef JABBER_WITH_APP
#include <jabber_app.hpp>
#endif // JABBER_WITH_APP

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

/**
 * @brief Because y-momentum is near 0, this absolute error is
 * employed for it.
 */
static constexpr double kAbsError = 1e-14;

/// Number of points to initialize + test at
static constexpr std::size_t kNumPts = 5;

/// Number of kTimes to test at
static constexpr std::size_t kNumTimes = 5;

/// Seed for randomizer
static constexpr int kSeed = 0;

/// Base flow params:
static constexpr double kRhoBar = 0.1792;
static constexpr double kPBar = 2000.0;
static const std::vector<double> kUBar = {600.0, 0.0, 450.0};
static constexpr double kGamma = 1.4;

/// Wave params:
static constexpr std::array<double, 2> kPAmps = {10.0, 5.0};
static constexpr std::array<double, 2> kFreqs = {1000.0, 1250.0};
static constexpr std::array<double, 2> kPhases = {M_PI/3, M_PI};
static constexpr std::array<char, 2> kSpeeds = {'S', 'F'};
static const std::array<std::vector<double>,2> 
                kWaveDirs = {std::vector<double>({1.0, 0.0, 0.0}),
                            std::vector<double>({1.0/3.0, 2.0/3.0, 2.0/3.0})};
/**
 * @brief Coordinates data, in XY XY ordering.
 * 
 * @details Same extent as 1D, in all directions.
 */
static const std::array<double, kNumPts*3> kCoords
                        = GenerateRandomArr<kNumPts*3>(kSeed,0.0,2.0);

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
struct AnalyticalSolution3D
{
private:
   static double PPrimeWave1(double x, double y, double z, double t)
   {
      return 10.0*std::cos((80.0*M_PI/19.0)*x + M_PI/3.0 
                                                      - 2000.0*M_PI*t);
   }
   static double PPrimeWave2(double x, double y, double z, double t)
   {
      return 5.0*std::cos((4.0*M_PI/3.0)*x + (8.0*M_PI/3.0)*(y+z) + M_PI
                                                        - 2500.0*M_PI*t);
   }
   static double PPrime(double x, double y, double z, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return PPrimeWave1(x,y,z,t);
      }
      else
      {
         return PPrimeWave1(x,y,z,t) + PPrimeWave2(x,y,z,t);
      }
   }
   static double Ux(double x, double y, double z, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 600.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,y,z,t));
      }
      else
      {
         return 600.0 + (1.0/(0.1792*125.0))*(-1.0*PPrimeWave1(x,y,z,t)
                                             + (1.0/3.0)*PPrimeWave2(x,y,z,t));
      }
   }
   static double Uy(double x, double y, double z, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 0.0;
      }
      else
      {
         return (1.0/(0.1792*125.0))*((2.0/3.0)*PPrimeWave2(x,y,z,t));
      }
   }
   static double Uz(double x, double y, double z, double t)
   {
      if constexpr (NumWaves == 1)
      {
         return 450.0;
      }
      else
      {
         return 450.0 + (1.0/(0.1792*125.0))*((2.0/3.0)*PPrimeWave2(x,y,z,t));
      }
   }
public:
   static double Rho(double x, double y, double z, double t)
   {
      return 0.1792 + (1.0/15625.0)*(PPrime(x,y,z,t));
   }
   static double RhoUx(double x, double y, double z, double t)
   {
      return Rho(x,y,z,t)*Ux(x,y,z,t);
   }
   static double RhoUy(double x, double y, double z, double t)
   {
      return Rho(x,y,z,t)*Uy(x,y,z,t);
   }
   static double RhoUz(double x, double y, double z, double t)
   {
      return Rho(x,y,z,t)*Uz(x,y,z,t);
   }
   static double RhoE(double x, double y, double z, double t)
   {
      return (2000.0/(1.4-1.0)) + (1.0/(1.4-1.0))*PPrime(x,y,z,t)
               + (1.0/2.0)*Rho(x,y,z,t)*(Ux(x,y,z,t)*Ux(x,y,z,t) 
                                       + Uy(x,y,z,t)*Uy(x,y,z,t)
                                       + Uz(x,y,z,t)*Uz(x,y,z,t));
   }
};

/**
 * @brief Check solution against analytical. Note that \p coords is in
 * XYZ XYZ ordering, while \p rhoU is in XX YY ZZ ordering.
 */
static void CheckSolution(std::span<const double> coords,
                           std::span<const double> rho,
                           std::span<const double> rhoU,
                           std::span<const double> rhoE,
                           double t, int num_waves)
{
   using function_t = std::function<double(double,double,double,double)>;
   function_t rho_exact, rhoUx_exact, rhoUy_exact, rhoUz_exact, rhoE_exact;

   if (num_waves == 1)
   {
      rho_exact = AnalyticalSolution3D<1>::Rho;
      rhoUx_exact = AnalyticalSolution3D<1>::RhoUx;
      rhoUy_exact = AnalyticalSolution3D<1>::RhoUy;
      rhoUz_exact = AnalyticalSolution3D<1>::RhoUz;
      rhoE_exact = AnalyticalSolution3D<1>::RhoE;
   }
   else if (num_waves == 2)
   {
      rho_exact = AnalyticalSolution3D<2>::Rho;
      rhoUx_exact = AnalyticalSolution3D<2>::RhoUx;
      rhoUy_exact = AnalyticalSolution3D<2>::RhoUy;
      rhoUz_exact = AnalyticalSolution3D<2>::RhoUz;
      rhoE_exact = AnalyticalSolution3D<2>::RhoE;
   }
   else
   {
      FAIL("Test does not support number of waves = " << num_waves);
   }

   for (std::size_t i = 0; i < kNumPts; i++)
   {
      const double x = coords[i*3];
      const double y = coords[i*3+1];
      const double z = coords[i*3+2];
      CAPTURE(x,y,z,t);
      CHECK_THAT(rho[i], WithinULP(rho_exact(x,y,z,t), kULP));
      CHECK_THAT(rhoU[i], WithinULP(rhoUx_exact(x,y,z,t), kULP));
      CHECK_THAT(rhoU[kNumPts+i], WithinAbs(rhoUy_exact(x,y,z,t), kAbsError));
      CHECK_THAT(rhoU[2*kNumPts+i], WithinULP(rhoUz_exact(x,y,z,t), kULP));
      CHECK_THAT(rhoE[i], WithinULP(rhoE_exact(x,y,z,t), kULP));
   }
}

TEST_CASE("3D flowfield computation via kernel", "[3D][Compute][Kernels]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      const double c_bar = std::sqrt(kGamma*kPBar/kRhoBar);

      std::vector<double> k_dot_x_p_phi(kNumPts*kNumWaves);
      std::vector<double> omega(kNumWaves);
      std::vector<double> mod_wave_dir(kNumWaves*3);

      // Initialize kernel variables
      for (int w = 0; w < kNumWaves; w++)
      {
         omega[w] = 2*M_PI*kFreqs[w];

         const std::vector<double> k_hat = kWaveDirs[w];
         const double mod_fac = (kSpeeds[w] == 'S' ? -1.0 : 1.0);

         mod_wave_dir[w] = mod_fac*k_hat[0];
         mod_wave_dir[kNumWaves+w] = mod_fac*k_hat[1];
         mod_wave_dir[2*kNumWaves+w] = mod_fac*k_hat[2];
         const double U_bar_dot_k_hat = kUBar[0]*k_hat[0] + kUBar[1]*k_hat[1]
                                                        + kUBar[2]*k_hat[2];

         const double k = (kSpeeds[w] == 'S' 
                                    ? omega[w]/(U_bar_dot_k_hat - c_bar)
                                    : omega[w]/(U_bar_dot_k_hat + c_bar));
         
         for (std::size_t i = 0; i < kNumPts; i++)
         {
            k_dot_x_p_phi[w*kNumPts + i] = k*(k_hat[0]*kCoords[i*3] 
                                              + k_hat[1]*kCoords[i*3+1]
                                              + k_hat[2]*kCoords[i*3+2])
                                           + kPhases[w];
         }
      }

      std::array<double, kNumPts> rho;
      std::array<double, kNumPts*3> rhoU;
      std::array<double, kNumPts> rhoE;

      // Evaluate kernel
      for (const double &time : kTimes)
      {
         // Compute
         ComputeKernel<3>(kNumPts, kRhoBar, kPBar, kUBar.data(), kGamma,
                           kNumWaves, kPAmps.data(), omega.data(), 
                           mod_wave_dir.data(), k_dot_x_p_phi.data(), time, 
                           rho.data(), rhoU.data(), rhoE.data());

         // Check solutions
         CheckSolution(kCoords, rho, rhoU, rhoE, time, kNumWaves);
      }
   }
}

TEST_CASE("3D flowfield computation via AcousticField", 
            "[3D][Compute][AcousticField]")
{
   const int kNumWaves = GENERATE(1,2);
   CAPTURE(kNumWaves);
   DYNAMIC_SECTION("Number of waves: " << kNumWaves)
   {
      // Build AcousticField
      AcousticField field(3, kCoords, kPBar, kRhoBar, kUBar, kGamma);

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

TEST_CASE("3D flowfield computation via app library", "[3D][Compute][App]")
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

      // Initialize AcousticField
      AcousticField field = InitializeAcousticField(config, kCoords, 3);

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
