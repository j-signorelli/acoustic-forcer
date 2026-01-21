#include <jabber.hpp>
#include <jabber_app.hpp>
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

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);

   // Option parser:
   cxxopts::Options options("jabber", 
      "Planar acoustic wave forcer for flow simulations using preCICE.");
   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
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
   AcousticField field(dim, coords, base_conf.p, base_conf.rho,
                        base_conf.U, base_conf.gamma);

   // Assemble vector of wave structs based on input source
   std::vector<Wave> all_waves;

   std::visit(
   overloads
   {
   [&](const SourceParams<SourceOption::SingleWave> &params_wave)
   {
      bool slow = (params_wave.speed == SpeedOption::Slow ? true : false);
      std::vector<double> k_hat(dim, 0.0);
      k_hat[0] = std::cos(params_wave.angle);
      if (dim > 1)
      {
         k_hat[1] = std::sin(params_wave.angle);
      }
      all_waves.emplace_back(params_wave.amp, params_wave.freq, 
                            params_wave.phase*M_PI/180.0, slow, k_hat);
   },
   [&](const SourceParams<SourceOption::WaveSpectrum> &params_waves)
   {
      for (int i = 0; i < params_waves.amps.size(); i++)
      {
         bool slow = (params_waves.speeds[i] == SpeedOption::Slow ? true : false);
         std::vector<double> k_hat(dim, 0.0);
         k_hat[0] = std::cos(params_waves.angles[i]);
         if (dim > 1)
         {
            k_hat[1] = std::sin(params_waves.angles[i]);
         }
         all_waves.emplace_back(params_waves.amps[i], params_waves.freqs[i],
                              params_waves.phases[i]*M_PI/180.0, slow, k_hat);
      }
   }
   }, source_conf);

   // Apply transfer function
   // TODO


   
   for (const Wave &wave : all_waves)
   {  
      // Add wave to acoustic field
      field.AddWave(wave);
   }

   // Finalize field initialization
   field.Finalize();

   std::cout << "Done!" << std::endl;

   double time = comp_conf.t0;
   double dt;

   // Compute acoustic forcing
   while (participant.isCouplingOngoing())
   {
      dt = participant.getMaxTimeStepSize();

      // Compute acoustic forcing
      field.Compute(time);

      // Send data
      participant.writeData(precice_conf.fluid_mesh_name, "rho",
                             vertex_ids, field.Density());
      for (int d = 0; d < dim; d++)
      {
         participant.writeData(precice_conf.fluid_mesh_name, 
                              "rhoV" + std::to_string(d+1),
                             vertex_ids, field.Momentum(d));
      }
      participant.writeData(precice_conf.fluid_mesh_name, "rhoE",
                             vertex_ids, field.Energy());       
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