#include "common.hpp"

#include <cmath>
#include <memory>
#include <random>
#include <fstream>
#include <ranges>
#include <algorithm>

using namespace jabber;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

using namespace jabber;

namespace jabber_app
{

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

void Normalize(std::span<const double> vec, std::span<double> norm_vec)
{
   double sum_sq = 0.0;
   for (std::size_t i = 0; i < vec.size(); i++)
   {
      const double val = vec[i];
      sum_sq += val*val;
   }

   const double mag = std::sqrt(sum_sq);
   for (std::size_t i = 0; i < vec.size(); i++)
   {
      norm_vec[i] = vec[i]/mag;
   }
}

void SourceParamsInitializer::operator()
   (const SourceParams<SourceOption::SingleWave> &sp)
{
   std::vector<double> k_hat(sp.direction.size(), 0.0);
   Normalize(sp.direction, k_hat);
   waves.emplace_back(sp.amp, sp.freq, sp.phase*M_PI/180.0, sp.speed, k_hat);
}

void SourceParamsInitializer::operator()
   (const SourceParams<SourceOption::WaveSpectrum> &sp)
{
   for (int i = 0; i < sp.amps.size(); i++)
   {
      std::vector<double> k_hat(sp.directions[i].size(), 0.0);
      Normalize(sp.directions[i], k_hat);
      waves.emplace_back(sp.amps[i], sp.freqs[i], sp.phases[i]*M_PI/180.0, 
                           sp.speeds[i], k_hat);
   }
}

void SourceParamsInitializer::operator()
   (const SourceParams<SourceOption::DigitalPSD> &sp)
{
   // Process input data into freqs, PSDs vector
   std::vector<double> d_freqs, d_psds;
   std::visit(
   overloads
   {
   [&](const PSDInputParams<PSDInputOption::Here> &input_params)
   {
      d_freqs = input_params.freqs;
      d_psds = input_params.psds;
   },
   [&](const PSDInputParams<PSDInputOption::FromCSV> &input_params)
   {
      std::ifstream is(input_params.file);
      if (!is.is_open())
      {
         throw std::invalid_argument("Cannot find PSD CSV file.");
      }
      for (std::string line; std::getline(is, line);)
      {
         auto row_view = std::ranges::views::split(line,std::string_view(","));
         auto row_it = row_view.begin();
         auto val_it = *row_it;
         d_freqs.push_back(std::stod(std::string(val_it.begin(), 
                                                   val_it.end())));
         val_it = *(++row_it);
         d_psds.push_back(std::stod(std::string(val_it.begin(), 
                                                   val_it.end())));
      }
   }
   }, sp.input_params);

   // Initialize the interpolated PSD curve
   std::unique_ptr<BasePSD> psd;
   if (sp.interp == InterpolationOption::PiecewiseLinear)
   {
      psd = std::make_unique<PWLinearPSD>(d_freqs, d_psds);
   }
   else if (sp.interp == InterpolationOption::PiecewiseLogLog)
   {
      psd = std::make_unique<PWLogLogPSD>(d_freqs, d_psds);
   }

   // Apply the discretization method for selecting center frequencies
   std::vector<double> freqs(sp.num_waves);
   const double min_freq = sp.min_disc_freq;
   const double max_freq = sp.max_disc_freq;
   std::visit(
   overloads
   {
   [&](const DiscMethodParams<DiscMethodOption::Uniform> &disc_params)
   {
      const double df = (max_freq - min_freq)/(freqs.size()-1);
      for (std::size_t i = 0; i < freqs.size(); i++)
      {
         freqs[i] = min_freq + df*i;
      }
   },
   [&](const DiscMethodParams<DiscMethodOption::UniformLog> &disc_params)
   {
      const double df = std::log10(max_freq/min_freq)/(freqs.size()-1);
      const double fac = std::pow(10.0, df);
      freqs[0] = min_freq;
      for (std::size_t i = 1; i < freqs.size(); i++)
      {
         freqs[i] = freqs[i-1]*std::pow(10.0, df);
      }
   },
   [&](const DiscMethodParams<DiscMethodOption::Random> &disc_params)
   {
      std::mt19937 gen(disc_params.seed);
      std::uniform_real_distribution<double> real_dist(min_freq, max_freq);
      for (std::size_t i = 0; i < freqs.size(); i++)
      {
         freqs[i] = real_dist(gen);
      }
   },
   [&](const DiscMethodParams<DiscMethodOption::RandomLog> &disc_params)
   {
      std::mt19937 gen(disc_params.seed);
      std::uniform_real_distribution<double> real_dist(std::log10(min_freq),
                                                       std::log10(max_freq));
      for (std::size_t i = 0; i < freqs.size(); i++)
      {
         freqs[i] = std::pow(10.0, real_dist(gen));
      }
   }
   }, sp.disc_params);

   // Sort the frequencies
   std::sort(freqs.begin(), freqs.end());

   // Compute the powers of each wave
   std::vector<double> powers(freqs.size());
   psd->Discretize(freqs, sp.int_method, powers);

   // Compute the amplitudes of each wave, with dimensionalization factor
   std::vector<double> amps(freqs.size());
   for (std::size_t i = 0; i < amps.size(); i++)
   {
      amps[i] = std::sqrt(2*powers[i])*sp.dim_fac;
   }

   // Compute the phases of each wave
   std::vector<double> phases(freqs.size());
   std::mt19937 phase_gen(sp.phase_seed);
   std::uniform_real_distribution<double> phase_dist(0,2*M_PI);
   for (std::size_t i = 0; i < phases.size(); i++)
   {
      phases[i] = phase_dist(phase_gen);
   }

   // Compute the normalized directions of each wave
   std::vector<std::vector<double>> k_hats(freqs.size());
   std::visit(
   overloads
   {
   [&](const DirectionParams<DirectionOption::Constant> &dir_params)
   {
      for (std::size_t i = 0; i < k_hats.size(); i++)
      {
         k_hats[i] = dir_params.direction;
      }
   },
   [&](const DirectionParams<DirectionOption::RandomXYAngle> &dir_params)
   {
      std::mt19937 dir_gen(dir_params.seed);
      std::uniform_real_distribution<double> dir_dist(
                                       dir_params.min_angle*M_PI/180.0,
                                       dir_params.max_angle*M_PI/180.0);
      for (std::size_t i = 0; i < k_hats.size(); i++)
      {
         k_hats[i].resize(3, 0.0);
         const double angle = dir_dist(dir_gen);
         k_hats[i][0] = std::cos(angle);
         k_hats[i][1] = std::sin(angle);
      }
   }
   }, sp.dir_params);

   // Finally, assemble the individual Wave structs
   for (std::size_t i = 0; i < sp.num_waves; i++)
   {
      const Wave w{amps[i], freqs[i], phases[i], sp.speed, k_hats[i]};
      waves.emplace_back(w);
   }
}

void SourceParamsInitializer::operator()
   (const SourceParams<SourceOption::WaveCSV> &sp)
{
   std::ifstream is(sp.file);
   if (!is.is_open())
   {
      throw std::invalid_argument("Wave CSV file not found.");
   }
   ReadWaves(is, waves);
}

AcousticField InitializeAcousticField(const ConfigInput &conf, 
                                       std::span<const double> coords,
                                       int dim)
{
   // Get relevant input metadata
   const BaseFlowParams &base_conf = conf.BaseFlow();
   const std::vector<SourceParamsVariant> &sources_conf = conf.Sources();

   // Initialize acoustic field
   AcousticField field(dim, coords, base_conf.p, base_conf.rho,
                        base_conf.U, base_conf.gamma);

   // Assemble vector of wave structs based on input source
   for (const SourceParamsVariant &source : sources_conf)
   {
      std::visit(SourceParamsInitializer{field.Waves()}, source);
   }

   // Finalize the acoustic field initialization
   field.Finalize();

   return std::move(field);

}                                        

} // namespace jabber_app
