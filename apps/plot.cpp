#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>
#include <matplot/matplot.h>

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

   matplot::figure_handle f = matplot::figure(true);
   f->size(1920, 1080);
   f->font_size(16);
   matplot::axes_handle ax = f->current_axes();
   ax->title("Wave Amplitude v. Frequency");
   ax->xlabel("Frequencies");
   ax->ylabel("Amplitudes");
   ax->font_size(16);
   ax->x_axis().label_font_size(16);
   ax->y_axis().label_font_size(16);
   if (loglog)
   {
      ax->x_axis().scale(matplot::axis_type::axis_scale::log);
      ax->y_axis().scale(matplot::axis_type::axis_scale::log);
   }

   matplot::line_handle line = ax->scatter(freqs, amps);
   line->marker_style("o");
   line->color("black");
   line->marker_size(8);
   line->marker_face(true);

   f->show();

   return 0;
}
