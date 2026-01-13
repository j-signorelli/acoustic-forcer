#include <precice/precice.hpp>
#include <cxxopts.hpp>
#include <jabber.hpp>

#include <iostream>

// Helper type for the std::visit (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

/// Print Jabber banner
void PrintBanner(std::ostream &out);

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);

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
   jabber::Config conf(config_file, &std::cout);

   // Initialize preCICE participant
   const jabber::Config::PreciceMeta &precice_conf = conf.Precice();
   precice::Participant participant(precice_conf.participant_name,
                                    precice_conf.config_file, 0, 1);

   // Get dimension of mesh to write forcing data onto
   int dim = participant.getMeshDimensions(precice_conf.fluid_mesh_name);

   // Assemble Waves based on config mode
   jabber::Waves waves(dim);

   std::visit(
   [&](const jabber::Config::SingleWaveMeta &w)
   {
      waves.SetNumWaves(1);
      waves.Amplitude(0) = w.amp;
      waves.Frequency(0) = w.freq;
      waves.Phase(0) = w.phase;

      // Compute wavenumber vector given angle
      
      std::vector<double> wave_number(dim,0.0);

      
   }, conf.Mode());

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