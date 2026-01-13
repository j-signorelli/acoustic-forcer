#include <precice/precice.hpp>
#include <cxxopts.hpp>
#include <jabber.hpp>

#include <iostream>
#include <cmath>

using namespace jabber;

// Helper type for the std::visit (https://en.cppreference.com/w/cpp/utility/variant/visit)
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
      ("f,file", "Config file.", cxxopts::value<std::string>())
      ("h,help", "Print usage information.");
   cxxopts::ParseResult result = options.parse(argc, argv);
   if (result.count("help"))
   {
      std::cout << options.help() << std::endl;
      return 0;
   }
   if (result.count("file") == 0)
   {
      std::cout << "Error: no config file specified." << std::endl;
      return 1;
   }
   
   // Parse config file
   std::string config_file = result["file"].as<std::string>();
   Config conf(config_file, &std::cout);

   const Config::BaseFlowMeta &base_conf = conf.BaseFlow();
   const Config::ModeMeta &mode_conf = conf.Mode();
   const Config::PreciceMeta &precice_conf = conf.Precice();

   // Initialize preCICE participant
   precice::Participant participant(precice_conf.participant_name,
                                    precice_conf.config_file, 0, 1);
   participant.initialize();

   // Get mesh information from fluid participant
   int dim = participant.getMeshDimensions(precice_conf.fluid_mesh_name);
   int vertex_size = participant.getMeshVertexSize(
                                          precice_conf.fluid_mesh_name);
   std::vector<double> coords(dim*vertex_size);
   std::vector<int> vertex_ids(vertex_size);
   participant.getMeshVertexIDsAndCoordinates(precice_conf.fluid_mesh_name,
                                              vertex_ids, coords);

   // Assemble Waves object based on config mode
   std::cout << "Assembling waves... ";
   AcousticField field(dim, coords);

   std::visit(
   [&](const Config::SingleWaveMeta &w)
   {
      waves.SetNumWaves(1);
      waves.Amplitude(0) = w.amp;
      waves.Frequency(0) = w.freq;
      waves.Phase(0) = w.phase;

      // Compute wavenumber vector given angle
      double cos_theta = std::cos(w.angle*M_PI/180.0);
      double sin_theta = std::sin(w.angle*M_PI/180.0);
      double c = std::sqrt(base_conf.gamma*base_conf.p/base_conf.rho);

      waves.WaveNumber(0,0) = w.freq*2*M_PI*cos_theta/
                              (base_conf.U*sin_theta + c);
      if (dim > 1)
      {
         waves.WaveNumber(1,0) = w.freq*2*M_PI*sin_theta/
                                 (base_conf.U*sin_theta + c);
      }
      if (dim == 3)
      {
         waves.WaveNumber(2,0) = 0.0;
      }                         
      
   }, mode_conf);

   // Finalize waves computation
   waves.Finalize();

   std::cout << "Done!" << std::endl;
   

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