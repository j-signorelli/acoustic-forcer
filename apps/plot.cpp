#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>

#include <iostream>
#include <algorithm>
#include <regex>
#include <fstream>

using namespace jabber;
using namespace jabber_app;

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);
   std::cout << "Jabber Spectra Plotter" << std::endl 
               << LINE << std::endl;

    // Option parser:
   cxxopts::Options options("jabber_plot", 
      "Generate a plot of the final wave spectra from a config file.");

   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
      ("l,log", "Plot on a log-log scale.",
                    cxxopts::value<bool>()->default_value("false"))
      ("h,help", "Print usage information.");

   cxxopts::ParseResult result = options.parse(argc, argv);
   
   std::string args_str = result.arguments_string();
   args_str = std::regex_replace(args_str, std::regex("\n"), "\n\t");
   std::cout << "Command Line Arguments\n\t" << args_str << std::endl 
               << LINE << std::endl;

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

   const bool loglog = result["log"].as<bool>();

   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);
   std::cout << LINE << std::endl;

   std::vector<Wave> waves;
   for (const SourceParamsVariant &spv : conf.Sources())
   {
      std::visit(SourceVisitor{waves}, spv);
   }

   std::vector<double> freqs(waves.size()), amps(waves.size());
   for (std::size_t i = 0; i < waves.size(); i++)
   {
      freqs[i] = waves[i].frequency;
      amps[i] = waves[i].amplitude;
   }

   std::FILE* gnuplot = popen("gnuplot", "w");

   if (loglog)
   {
      std::fprintf(gnuplot, "set logscale xy\n");
   }

   std::fprintf(gnuplot, "unset key\n");
   
   std:fprintf(gnuplot, "set xlabel 'Frequency'\n");
   std::fprintf(gnuplot, "set ylabel 'Wave Amplitude'\n");
   std::fprintf(gnuplot, "plot '-' with points pt 5\n");
   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      std::fprintf(gnuplot, "%s", std::format("{} {}\n", 
                                       freqs[i], amps[i]).c_str());
   }
   std::fprintf(gnuplot, "e\n");
   std::cout << "Enter to close plot...";
   std::fflush(gnuplot);
   std::cin.get();
   pclose(gnuplot);

   return 0;
}
