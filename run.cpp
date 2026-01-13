#include <precice/precice.hpp>
#include <cxxopts.hpp>

#include <iostream>


int main(int argc, char *argv[])
{ 
   cxxopts::Options options("AcousticParticipant", 
      "Planar acoustic wave forcer for flow simulations.");

   options.add_options()
      ("f,file", "Acoustic data CSV file.", cxxopts::value<std::string>())
      ("h,help", "Print usage information.");

   cxxopts::ParseResult result = options.parse(argc, argv);

   if (result.count("help"))
   {
      std::cout << options.help() << std::endl;
      return 0;
   }

   return 0;
}
