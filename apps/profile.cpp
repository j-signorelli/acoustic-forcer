#include <jabber.hpp>
#include <jabber_app.hpp>
#include <cxxopts.hpp>

#include <iostream>
#include <cmath>
#include <regex>
#include <chrono>
#include <random>

using namespace jabber;
using namespace jabber_app;

/// Create a dummy grid.
void CreateGrid(const int dim, const int num_pts_d, const double extent, 
                  std::vector<double> &coords);


int main(int argc, char *argv[])
{
   PrintBanner(std::cout);
   std::cout << "Jabber Profiler Tool" << std::endl
               << LINE << std::endl;

   // Option parser:
   cxxopts::Options options("jabber_profile", 
      "Simple profiler tool to obtain execution times of a given config file \
        and grid.");

   options.add_options()
      ("c,config", "Config file.", cxxopts::value<std::string>())
      ("d,dim", "Grid dimension (1,2,3).", 
                        cxxopts::value<int>()->default_value("3"))
      ("n,num_points", "Number grid points in each dimension.",
                        cxxopts::value<std::size_t>()->default_value("100"))
      ("e,extent", "Grid extent in each direction (such that domain is "
                     "[0,extent]^dim).",
                        cxxopts::value<double>()->default_value("1.0"))
      ("p,passes", "Number of passes to Compute() to profile, using "
                   "randomized times.",
                        cxxopts::value<int>()->default_value("10000"))
      ("w,warmup", "Number of warmup passes to Compute(), using randomized "
                   "times.", cxxopts::value<int>()->default_value("1000"))
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

   const int dim = result["dim"].as<int>();
   const std::size_t num_pts_d = result["num_points"].as<std::size_t>();
   const std::size_t num_pts_total = std::pow(num_pts_d,dim);
   const double extent = result["extent"].as<double>();
   const int passes = result["passes"].as<int>();
   const int warmup_passes = result["warmup"].as<int>();

   // Parse config file
   std::string config_file = result["config"].as<std::string>();
   TOMLConfigInput conf(config_file, &std::cout);
   std::cout << LINE << std::endl;

   // Output grid information to console
   std::cout << "Grid\n";
   std::cout << "\tDimension: ";
   for (int d = 0; d < dim; d++)
   {
      std::cout << num_pts_d << (d+1==dim ? "" : "x");
   }
   std::cout << std::endl;
   std::cout << "\tExtents: ";
   for (int d = 0; d < dim; d++)
   {
      std::cout << "[0," << extent << "]" << ((d + 1 == dim) ? "" : "x");
   }
   std::cout << std::endl;
   std::cout << "\tNumber of points: " << num_pts_total << std::endl;
   std::cout << "\tSpacing: " << (extent/(num_pts_d-1.0)) << std::endl;

   // Create a simple [0,1]^dim grid
   std::vector<double> coords(num_pts_total*dim);
   CreateGrid(dim, num_pts_d, extent, coords);

   // Initialize AcousticField
   AcousticField field = InitializeAcousticField(conf, coords, dim);


   // Create an array of randomized times
   std::mt19937 gen(0);
   std::uniform_real_distribution<double> real_dist(0,1);
   std::vector<double> time_rand(passes+warmup_passes);
   for (int i = 0; i < time_rand.size(); i++)
   {
      time_rand[i] = real_dist(gen);
   }

   // Run profiling loop
   std::vector<std::chrono::duration<double, std::micro>> 
      compute_times(passes+warmup_passes);
   for (int i = 0; i < time_rand.size(); i++)
   {
      const std::chrono::time_point<std::chrono::steady_clock> start =
        std::chrono::steady_clock::now();
      field.Compute(time_rand[i]);
      const std::chrono::time_point<std::chrono::steady_clock> end =
        std::chrono::steady_clock::now();

      compute_times[i] = 
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      
      if (i < warmup_passes)
      {
         std::cout << "Warmup Pass #" << (i+1) << ": " << compute_times[i]
                   << std::endl;
      }
      else
      {
         std::cout << "Pass #" << (i+1-warmup_passes) << ": "
                   << compute_times[i] << std::endl;
      }
   }

   // Compute the average compute time after warmups
   std::chrono::duration<double, std::micro> total_dur(0.0);
   for (int i = 0; i < passes; i++)
   {
      total_dur += compute_times[i+warmup_passes];
   }
   std::chrono::duration<double, std::micro> ave_dur = total_dur/passes;

   std::cout << LINE << std::endl << "Average Compute() Time: " << ave_dur << std::endl;
   return 0;
}

void CreateGrid(const int dim, const int num_pts_d, const double extent, 
                  std::vector<double> &coords)
{
   const double h = extent/(num_pts_d-1);
   int num_pts_total = 1;
   for (int d = 1; d < dim; d++)
   {
      num_pts_total *= num_pts_d;
   }
   coords.resize(num_pts_total*dim);

   if (dim == 1)
   {
      for (std::size_t i = 0; i < num_pts_d; i++)
      {
         coords[i] = h*i;
      }
   }
   else if (dim == 2)
   {
      for (std::size_t i = 0; i < num_pts_d; i++)
      {
         const double x = h*i;
         for (std::size_t j = 0; j < num_pts_d; j++)
         {
            const std::size_t completed_pts = i*num_pts_d + j;
            coords[completed_pts*dim] = x;
            coords[completed_pts*dim+1] = h*j;
         }
      }
   }
   else // dim == 3
   {
      for (std::size_t i = 0; i < num_pts_d; i++)
      {
         const double x = h*i;
         for (std::size_t j = 0; j < num_pts_d; j++)
         {
            const double y = h*j;
            for (std::size_t k = 0; k < num_pts_d; k++)
            {
               const std::size_t completed_pts = i*num_pts_d*num_pts_d 
                                                   + j*num_pts_d + k;
               coords[completed_pts*dim] = x;
               coords[completed_pts*dim+1] = y;
               coords[completed_pts*dim+2] = h*k;
            }
         }
      }
   }
}
