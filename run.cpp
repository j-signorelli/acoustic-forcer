#include <precice/precice.hpp>
#include <cxxopts.hpp>
#include <jabber.hpp>

#include <iostream>

void PrintBanner(std::ostream &out);

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);

   cxxopts::Options options("Jabber", 
      "Planar acoustic wave forcer for flow simulations, using preCICE.");

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
   
   std::string config_file = result["file"].as<std::string>();
   jabber::Config conf(config_file, &std::cout);

   

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