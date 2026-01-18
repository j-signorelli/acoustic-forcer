#include "config_input.hpp"

#include <jabber.hpp>
#include <precice/precice.hpp>
#include <cxxopts.hpp>

#include <iostream>
#include <cmath>

using namespace jabber;
using namespace jabber_app;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

/// Print Jabber banner
void PrintBanner(std::ostream &out);

/**
 * @brief Compute wavenumber vector \p k for an angled disturbance in the
 * xy-plane in freestream \f$U_\infty\f$ in the x-direction.
 * 
 * @details Specifically, evaluate
 * \f[
 * \alpha=\frac{2\pi f\cos(\theta)}{U_\infty\cos(\theta)\pm c_\infty},
 * \f]
 * and
 * \f[
 * \beta=\frac{2\pi f\sin(\theta)}{U_\infty\cos(\theta)\pm c_\infty}
 * \f]
 * for \f$\vec{k}=\left[\alpha,\beta,0\right]\f$.
 * 
 * @param dim           Spatial dimension.
 * @param theta         Wave angle in degrees w.r.t x-axis. CCW positive.
 * @param u_infty       Freestream velocity.
 * @param c_infty       Freestream speed of sound.
 * @param freq          Frequency (not angular).
 * @param slow          true if slow, false if fast.        
 * @returns k           Wavenumber vector.
 */
void ComputeWavenumber(int dim, double theta, double u_infty, double c_infty,
                        double freq, bool slow, std::vector<double> &k);

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);

   // Option parser:
   cxxopts::Options options("jabber", 
      "Planar acoustic wave forcer for flow simulations using preCICE.");
   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>()),
      ("h,help", "Print usage information.");
   cxxopts::ParseResult result = options.parse(argc, argv);
   if (result.count("help"))
   {
      std::cout << options.help() << std::endl;
      return 0;
   }
   if (result.count("config") == 0)
   {
      std::cout << "Error: no config file specified." << std::endl;
      return 1;
   }
   
   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);

   // Get the input metadata
   const BaseFlowParams &base_conf = conf.BaseFlow();
   const SourceParamsVariant &source_conf = conf.Source();
   const CompParams &comp_conf = conf.Comp();
   const PreciceParams &precice_conf = conf.Precice();

   // Initialize preCICE participant
   precice::Participant participant(precice_conf.participant_name,
                                    precice_conf.config_file, 0, 1);
   participant.setMeshAccessRegion(precice_conf.fluid_mesh_name,
                                    precice_conf.mesh_access_region);
   participant.initialize();

   // Get mesh information from fluid participant
   int dim = participant.getMeshDimensions(precice_conf.fluid_mesh_name);
   int vertex_size = participant.getMeshVertexSize(
                                          precice_conf.fluid_mesh_name);
   std::vector<double> coords(dim*vertex_size);
   std::vector<int> vertex_ids(vertex_size);
   participant.getMeshVertexIDsAndCoordinates(precice_conf.fluid_mesh_name,
                                              vertex_ids, coords);

   // Assemble AcousticField object
   std::cout << "Assembling acoustic field data... ";
   AcousticField field(dim, coords);
   
   double c_infty = std::sqrt(base_conf.gamma*base_conf.p/base_conf.rho);

   // Assemble vector of wave structs based on input source
   std::vector<Wave> all_waves;
   std::visit(
   overloads
   {
   [&](const SourceParams<SourceOption::SingleWave> &params_wave)
   {
      Wave wave{params_wave.amp, params_wave.freq, params_wave.phase};
      
      bool slow = (params_wave.speed == SpeedOption::Slow ? true : false);
      ComputeWavenumber(dim, params_wave.angle, base_conf.U, c_infty, 
                        params_wave.freq, slow, wave.k);

      all_waves.emplace_back(wave);
   },
   [&](const SourceParams<SourceOption::WaveSpectrum> &params_waves)
   {
      std::vector<double> k(dim);
      for (int i = 0; i < params_waves.amps.size(); i++)
      {
         std::cout << "Adding wave " << i << std::endl;
         bool slow = (params_waves.speeds[i] == SpeedOption::Slow ? 
                        true : false);
         ComputeWavenumber(dim, params_waves.angles[i], base_conf.U, c_infty, 
                        params_waves.freqs[i], slow, k);
         
         all_waves.emplace_back(params_waves.amps[i], params_waves.freqs[i],
                                 params_waves.phases[i], k);
      }
   }
   }, source_conf);

   // Apply transfer function



   // Add all waves to AcousticField
   for (const Wave &wave : all_waves)
   {
      field.AddWave(wave);
   }

   // Finalize field initialization
   field.Finalize();

   std::cout << "Done!" << std::endl;
   
   // Initialize data vectors
   // p' must always be computed
   std::vector<double> p_prime(vertex_size);
   std::vector<double> rho, rhoV, rhoE;

   for (const DataOption &data : comp_conf.data)
   {
      switch (data)
      {
         case DataOption::Density:
            rho.resize(vertex_size);
            break;
         case DataOption::Momentum:
            rhoV.resize(vertex_size*dim);
            break;
         case DataOption::Energy:
            rhoE.resize(vertex_size);
            break;
      }
   }

   double time = comp_conf.t0;
   double dt;

   // Compute acoustic forcing
   while (participant.isCouplingOngoing())
   {
      dt = participant.getMaxTimeStepSize();

      // Compute acoustic forcing
      field.Compute(time, p_prime);

      // Compute + send data fields
      for (const DataOption &data : comp_conf.data)
      {
         switch (data)
         {
            case DataOption::PressurePerturbation:
               participant.writeData(precice_conf.fluid_mesh_name, "p'",
                                       vertex_ids, p_prime);
               break;
         }
        
      }
      participant.advance(dt);
      time += dt;
   }
   
   participant.finalize();

   return 0;
}

void PrintBanner(std::ostream &out)
{
   constexpr std::string_view banner = R"(
      /-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\|/-\
      |   ________  ____    ______  ______    _____ ______      |
      \  (___  ___)(    )  (_   _ \(_   _ \  / ___/(   __ \     /
      -      ) )   / /\ \    ) (_) ) ) (_) )( (__   ) (__) )    -
      /     ( (   ( (__) )   \   _/  \   _/  ) __) (    __/     \
      |  __  ) )   )    (    /  _ \  /  _ \ ( (     ) \ \  _    |
      \ ( (_/ /   /  /\  \  _) (_) )_) (_) ) \ \___( ( \ \_))   /
      -  \___/   /__(  )__\(______/(______/   \____\)_) \__/    -
      /                                                         \
      |                                                         |
      \-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/|\-/)";

   out << banner << std::endl << std::endl;
}

void ComputeWavenumber(int dim, double theta, double u_infty, double c_infty,
                        double freq, bool slow, std::vector<double> &k)
{
   k.resize(dim, 0.0);

   double cos_theta = std::cos(theta*M_PI/180.0);
   double sin_theta = std::sin(theta*M_PI/180.0);

   double denom = u_infty*cos_theta + (slow ? -c_infty : c_infty);

   k[0] = freq*2*M_PI*cos_theta/denom;
   if (dim > 1)
   {
      k[1] = freq*2*M_PI*sin_theta/denom;
   }
}