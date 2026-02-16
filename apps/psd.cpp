#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>
#include <pocketfft_hdronly.h>

#include <iostream>
#include <cmath>
#include <regex>
#include <fstream>

using namespace jabber;
using namespace jabber_app;

int main(int argc, char *argv[])
{
   PrintBanner(std::cout);
   std::cout << "Jabber PSD" << std::endl
               << LINE << std::endl;

   // Option parser:
   cxxopts::Options options("jabber_psd", 
      "Compute and plot a PSD from a probe of the exact "
      "flowfield computed by Jabber using Welch's method.");

   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
      ("d,dt", "Timestep to use.", 
         cxxopts::value<double>()->default_value("7.459237222374841e-9"))
      ("n,num-timesteps", "Number of timesteps to run to.", 
         cxxopts::value<std::size_t>()->default_value("1000000"))
      ("p,n-per-seg", "Number of points in each segment.",
         cxxopts::value<std::size_t>(256))
      ("f,file", "Name of file to write pressure data to.",
         cxxopts::value<std::string>()->default_value("pressures.txt"))
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

   const double dt = result["dt"].as<double>();
   const double nt = result["num-timesteps"].as<std::size_t>();
   const std::string file_name = result["file"].as<std::string>();

   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);
   std::cout << LINE << std::endl;
   std::vector<double> coords = {0.1, 0.1, 0.1};

   // Initialize AcousticField
   AcousticField field = InitializeAcousticField(conf, coords, 3);

   double time = 0.0;
   const double c_sq = conf.BaseFlow().gamma*conf.BaseFlow().p/conf.BaseFlow().rho;

   // Run through all times, storing pressures at each timestep
   std::vector<double> pressures(nt);
   for (int i = 0; i < nt; i++)
   {
      field.Compute(time);
      time += dt;
      const double p = conf.BaseFlow().p + c_sq*(field.Density()[0] - conf.BaseFlow().rho);
      pressures[i] = p;
   }
   
   // Compute the DFT
   // std::vector<std::complex<double>> spectrum(nt/2+1);
   // pocketfft::r2c({nt}, sizeof(double), sizeof(std::complex<double>), 0,
   //                pocketfft::FORWARD, pressures.data(), spectrum.data(), 1.0);
   
   
   

   return 0;
}
