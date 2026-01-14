#include <precice/precice.hpp>
#include <cxxopts.hpp>
#include <jabber.hpp>

#include <iostream>
#include <cmath>

using namespace jabber;

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
   Config conf(config_file, &std::cout);

   // Get the input metadata
   const Config::BaseFlowMeta &base_conf = conf.BaseFlow();
   const Config::SourceMeta &mode_conf = conf.Source();
   const Config::CompMeta &comp_conf = conf.Comp();
   const Config::PreciceMeta &precice_conf = conf.Precice();

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

   // Assemble AcousticField object based on config mode
   std::cout << "Assembling acoustic field data... ";
   AcousticField field(dim, coords);

   std::visit(
   [&](const Config::SingleWaveMeta &w)
   {
      Wave wave{w.amp, w.freq, w.phase};
      wave.k.resize(dim,0.0);

      // Compute wavenumber vector, given angle
      double cos_theta = std::cos(w.angle*M_PI/180.0);
      double sin_theta = std::sin(w.angle*M_PI/180.0);
      double c = std::sqrt(base_conf.gamma*base_conf.p/base_conf.rho);

      wave.k[0] = w.freq*2*M_PI*cos_theta/(base_conf.U*sin_theta + c);
      if (dim > 1)
      {
         wave.k[1] = w.freq*2*M_PI*sin_theta/(base_conf.U*sin_theta + c);
      }
      
      field.AddWave(wave);
      
   }, mode_conf);

   // Finalize field initialization
   field.Finalize();

   std::cout << "Done!" << std::endl;
   
   std::vector<double> p_prime(vertex_size);
   double time = comp_conf.t0;
   double dt;

   // Compute acoustic forcing
   while (participant.isCouplingOngoing())
   {
      dt = participant.getMaxTimeStepSize();
      field.Compute(time, p_prime);
      participant.writeData(precice_conf.fluid_mesh_name, "p'", vertex_ids,
                                 p_prime);
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
