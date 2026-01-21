#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>
//#include <mfem.hpp>

#include <iostream>
#include <algorithm>
#include <regex>

using namespace jabber;
using namespace jabber_app;

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);
   std::cout << "Jabber Acoustic Field Visualizer" << std::endl << std::endl;

    // Option parser:
   cxxopts::Options options("jabber_viz", 
      "Visualizer of acoustic field from config file using MFEM w/ GLVis.");

   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
      ("h,help", "Print usage information.");

   cxxopts::ParseResult result = options.parse(argc, argv);

   std::string args_str = result.arguments_string();
   args_str = std::regex_replace(args_str, std::regex("\n"), "\n\t");
   std::cout << "Command Line Arguments\n\t" << args_str << std::endl;

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


   return 0;
}
