#include "common.hpp"

#include <cmath>
#include <memory>
#include <random>

using namespace jabber;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

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

std::vector<std::pair<double,double>> ReadPSDFromCSV(std::string file_name)
{

}

std::vector<Wave>
InitializeFromPSD(const SourceParams<SourceOption::DigitalPSD> &source_params)
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
      std::vector<std::pair<double, double>> csv_data = 
                                          ReadPSDFromCSV(input_params.file);
      d_freqs.reserve(csv_data.size());
      d_psds.reserve(csv_data.size());
      for (const std::pair<double, double> &row : csv_data)
      {
         d_freqs.push_back(row.first);
         d_psds.push_back(row.second);
      }
   }
   }, source_params.input_params);

   // Initialize the interpolated PSD curve
   std::unique_ptr<BasePSD> psd;
   if (source_params.interp == InterpolationOption::PiecewiseLinear)
   {
      psd = std::make_unique<PWLinearPSD>(d_freqs, d_psds);
   }
   else if (source_params.interp == InterpolationOption::PiecewiseLogLog)
   {
      psd = std::make_unique<PWLogLogPSD>(d_freqs, d_psds);
   }

   // Apply the discretization method for selecting center frequencies
   std::vector<double> freqs(source_params.num_waves);
   const double min_freq = source_params.min_disc_freq;
   const double max_freq = source_params.max_disc_freq;
   std::visit(
   overloads
   {
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
   }, source_params.disc_params);


   // Compute the powers of each wave
   std::vector<double> powers(freqs.size());
   psd->Discretize(freqs, source_params.int_method, powers);

   /// @todo Apply transfer function
   // Compute the amplitudes of each wave, with dimensionalization factor
   std::vector<double> amps(freqs.size());
   for (std::size_t i = 0; i < amps.size(); i++)
   {
      amps[i] = std::sqrt(2*powers[i])*source_params.dim_fac;
   }

   // Compute the phases of each wave
   std::vector<double> phases(freqs.size());
   std::mt19937 phase_gen(source_params.phase_seed);
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

   },
   [&](const DirectionParams<DirectionOption::RandomXYAngle> &dir_params)
   {

   }
   }, source_params.dir_params);

   // Finally, assemble the individual Wave structs
   std::vector<Wave> waves(powers.size());
   for (std::size_t i = 0; i < source_params.num_waves; i++)
   {
      const Wave w{amps[i], freqs[i], phases[i], 
                     source_params.speed, k_hats[i]};
      waves.emplace_back(w);
   }

   return std::move(waves);
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
   std::vector<Wave> all_waves;

   for (const SourceParamsVariant &source : sources_conf)
   {
      std::visit(
      overloads
      {
      [&](const SourceParams<SourceOption::SingleWave> &params_wave)
      {
         std::vector<double> k_hat(dim, 0.0);
         Normalize(params_wave.direction, k_hat);
         all_waves.emplace_back(params_wave.amp, params_wave.freq, 
                                 params_wave.phase*M_PI/180.0, 
                                 params_wave.speed, k_hat);
      },
      [&](const SourceParams<SourceOption::WaveSpectrum> &params_waves)
      {
         for (int i = 0; i < params_waves.amps.size(); i++)
         {
            std::vector<double> k_hat(dim, 0.0);
            Normalize(params_waves.directions[i], k_hat);
            all_waves.emplace_back(params_waves.amps[i], params_waves.freqs[i],
                                    params_waves.phases[i]*M_PI/180.0, 
                                    params_waves.speeds[i], k_hat);
         }
      },
      [&](const SourceParams<SourceOption::DigitalPSD> &params_psd)
      {
         
      }
      }, source);
   }

   for (const Wave &wave : all_waves)
   {  
      // Add wave to acoustic field
      field.AddWave(wave);
   }

   // Finalize the acoustic field initialization
   field.Finalize();

   return std::move(field);

}                                        

} // namespace jabber_app
