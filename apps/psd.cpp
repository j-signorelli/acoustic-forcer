#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>
#include <pocketfft_hdronly.h>
#include <matplot/matplot.h>

#include <iostream>
#include <cmath>
#include <regex>
#include <fstream>

using namespace jabber;
using namespace jabber_app;

double HammingWindow(std::size_t N, std::size_t n);

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
         cxxopts::value<double>()->default_value("7.459237222374841e-7"))
      ("n,num-timesteps", "Number of timesteps to run to.", 
         cxxopts::value<std::size_t>()->default_value("1000000"))
      ("s,nperseg", "Number of points in each segment.",
         cxxopts::value<std::size_t>()->default_value("256"))
      ("o,overlap", "Percent overlap of segments.", 
         cxxopts::value<double>()->default_value("0.5"))
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

   const double dt = result["dt"].as<double>();
   const std::size_t nt = result["num-timesteps"].as<std::size_t>();
   const std::size_t nperseg = result["nperseg"].as<std::size_t>();
   const std::size_t overlap = result["overlap"].as<double>();
   const bool loglog = result["log"].as<bool>();

   if (nperseg > nt)
   {
      throw std::invalid_argument("nperseg must be less than nt!");
      return 1;
   }

   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);
   std::cout << LINE << std::endl;
   std::vector<double> coords = {0.1, 0.1, 0.1};

   // Initialize AcousticField
   AcousticField field = InitializeAcousticField(conf, coords, 3);

   double time = 0.0;
   const double c_sq = conf.BaseFlow().gamma*
                           conf.BaseFlow().p/conf.BaseFlow().rho;

   // Initialize vector of all pressures.
   std::vector<double> p_prime(nt, 0.0);

   // Run through all times, storing pressures at each timestep
   for (int i = 0; i < nt; i++)
   {
      field.Compute(time);
      time += dt;
      p_prime[i] = c_sq*(field.Density()[0] - conf.BaseFlow().rho);
   }

   const std::size_t shift = nperseg - std::round(overlap*nperseg);
   const std::size_t num_segs = 1+(nt-nperseg)/shift;

   // Compute W factor for modified periodogram + assemble window fxn
   std::vector<double> window(nperseg);
   double sum_W_sq = 0.0;
   for (std::size_t w = 0; w < nperseg; w++)
   {
      const double w_val = HammingWindow(nperseg, w);
      window[w] = w_val;
      sum_W_sq += w_val*w_val;
   }

   // Compute the DFT for each segment k
   const double fs = 1.0/dt;
   std::vector<double> xw(nperseg);
   std::vector<std::complex<double>> dft(nperseg/2+1);
   std::vector<double> psd(nperseg/2+1, 0.0);                
   for (std::size_t k = 0; k < num_segs; k++)
   {
      // Compute pressure * window
      for (std::size_t w = 0; w < nperseg; w++)
      {
         xw[w] = p_prime[k*shift+w]*window[w];
      }

      // Compute DFT of each segment
      pocketfft::r2c({nperseg}, {sizeof(double)}, 
                     {sizeof(std::complex<double>)},
                     0, pocketfft::FORWARD, xw.data(), 
                     dft.data(), 1.0);

      // Compute modified periodogram of DFT + add averaged contribution 
      // of this segment k to PSD
      for (std::size_t i = 0; i < nperseg/2+1; i++)
      {
         const double mod_p_i = std::norm(dft[i])/(sum_W_sq*fs);
         psd[i] += mod_p_i/num_segs;
      }
   }

   // Apply one-sided correction
   const std::size_t up_to = (nperseg % 2 == 1) ? nperseg/2+1 : nperseg/2;
   for (std::size_t i = 1; i < up_to; i++)
   {
      psd[i] *= 2;
   }

   // Get the frequencies
   std::vector<double> freqs(psd.size());
   for (std::size_t i = 0; i < nperseg/2+1; i++)
   {
      freqs[i] = i*fs/nperseg;
   }
   
   // Plot
   matplot::figure_handle f = matplot::figure(true);
   f->size(1920, 1080);
   f->font_size(16);
   matplot::axes_handle ax = f->current_axes();
   ax->title("Power Spectral Density");
   ax->xlabel("Frequencies");
   ax->font_size(16);
   ax->x_axis().label_font_size(16);
   ax->y_axis().label_font_size(16);
   if (loglog)
   {
      ax->x_axis().scale(matplot::axis_type::axis_scale::log);
      ax->y_axis().scale(matplot::axis_type::axis_scale::log);

      // Remove the DC component altogether
      freqs.erase(freqs.begin());
      psd.erase(psd.begin());
   }

   matplot::line_handle line = ax->scatter(freqs, psd);
   line->marker_style("o");
   line->color("black");
   line->marker_size(8);
   line->marker_face(true);

   f->show();

   return 0;
}



double HammingWindow(std::size_t N, std::size_t n)
{
   constexpr double a0 = 25.0/46.0;
   return a0 + (1-a0)*std::cos(2*M_PI*n/N);
}