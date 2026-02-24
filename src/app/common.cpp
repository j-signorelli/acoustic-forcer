#include "common.hpp"

#include <cmath>
#include <memory>
#include <random>
#include <fstream>
#include <ranges>
#include <algorithm>
#include <type_traits>

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


void InputXYVisitor::operator()
   (const InputXYParams<InputXYOption::Here> &ip)
{
   x = ip.x;
   y = ip.y;
}

void InputXYVisitor::operator()
   (const InputXYParams<InputXYOption::FromCSV> &ip)
{
   std::ifstream is(ip.file);
   if (!is.is_open())
   {
      throw std::invalid_argument(std::format("Cannot find XY-data CSV file "
                                              "'{}'.", ip.file));
   }
   for (std::string line; std::getline(is, line);)
   {
      auto row_view = std::ranges::views::split(line,std::string_view(","));
      auto row_it = row_view.begin();
      auto val_it = *row_it;
      x.push_back(std::stod(std::string(val_it.begin(), 
                                                val_it.end())));
      val_it = *(++row_it);
      y.push_back(std::stod(std::string(val_it.begin(), 
                                                val_it.end())));
   }
}

template<typename T>
void FunctionVisitor<T>::operator()
   (const FunctionParams<FunctionOption::PiecewiseLinear> &fp)
{
   std::vector<double> x, y;
   std::visit(InputXYVisitor{x,y}, fp.input_xy);

   if constexpr (std::is_same_v<T, Function>)
   {
      T_ptr = std::make_unique<PWLinear>(x,y);
   }
   else
   {
      T_ptr = std::make_unique<PWLinearPSD>(x,y);
   }
}

template<typename T>
void FunctionVisitor<T>::operator()
   (const FunctionParams<FunctionOption::PiecewiseLogLog> &fp)
{
   std::vector<double> x, y;
   std::visit(InputXYVisitor{x,y}, fp.input_xy);

   if constexpr (std::is_same_v<T, Function>)
   {
      T_ptr = std::make_unique<PWLogLog>(x,y);
   }
   else
   {
      T_ptr = std::make_unique<PWLogLogPSD>(x,y);
   }
}

// Explicitly initialize for Function and BasePSD.
template class FunctionVisitor<Function>;
template class FunctionVisitor<BasePSD>;

void DiscMethodVisitor::operator()
   (const DiscMethodParams<DiscMethodOption::Uniform> &dp)
{
   const double df = (max_freq - min_freq)/(freqs.size()-1);
   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      freqs[i] = min_freq + df*i;
   }
}

void DiscMethodVisitor::operator()
   (const DiscMethodParams<DiscMethodOption::UniformLog> &dp)
{
   const double log_df = std::log10(max_freq/min_freq)/(freqs.size()-1);
   const double log_min_freq = std::log10(min_freq);
   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      freqs[i] = std::pow(10.0, std::log10(min_freq) + log_df*i);
   }
}

void DiscMethodVisitor::operator()
   (const DiscMethodParams<DiscMethodOption::Random> &dp)
{
   std::mt19937 gen(dp.seed);
   std::uniform_real_distribution<double> real_dist(min_freq, max_freq);
   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      freqs[i] = real_dist(gen);
   }
}

void DiscMethodVisitor::operator()
   (const DiscMethodParams<DiscMethodOption::RandomLog> &dp)
{
   std::mt19937 gen(dp.seed);
   std::uniform_real_distribution<double> real_dist(std::log10(min_freq),
                                                      std::log10(max_freq));
   for (std::size_t i = 0; i < freqs.size(); i++)
   {
      freqs[i] = std::pow(10.0, real_dist(gen));
   }
}

void DirectionVisitor::operator()
   (const DirectionParams<DirectionOption::Constant> &dp)
{
   for (std::size_t i = 0; i < k_hats.size(); i++)
   {
      k_hats[i] = dp.direction;
   }
}

void DirectionVisitor::operator()
   (const DirectionParams<DirectionOption::RandomXYAngle> &dp)
{
   std::mt19937 dir_gen(dp.seed);
   std::uniform_real_distribution<double> dir_dist(
                                    dp.min_angle*M_PI/180.0,
                                    dp.max_angle*M_PI/180.0);
   for (std::size_t i = 0; i < k_hats.size(); i++)
   {
      k_hats[i].resize(3, 0.0);
      const double angle = dir_dist(dir_gen);
      k_hats[i][0] = std::cos(angle);
      k_hats[i][1] = std::sin(angle);
   }
}

void SourceVisitor::operator()
   (const SourceParams<SourceOption::SingleWave> &sp)
{
   std::vector<double> k_hat(sp.direction.size(), 0.0);
   Normalize(sp.direction, k_hat);
   waves.emplace_back(sp.amp, sp.freq, sp.phase*M_PI/180.0, sp.speed, k_hat);
}

void SourceVisitor::operator()
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

void SourceVisitor::operator()
   (const SourceParams<SourceOption::PSD> &sp)
{
   // Process input PSD
   std::unique_ptr<BasePSD> psd;
   std::visit(FunctionVisitor{psd}, sp.input_psd);

   // Apply the discretization method for selecting center frequencies
   std::vector<double> freqs(sp.num_waves);
   const double min_freq = sp.min_disc_freq;
   const double max_freq = sp.max_disc_freq;
   std::visit(DiscMethodVisitor{min_freq,max_freq, freqs}, sp.disc_params);

   // Sort the frequencies
   std::sort(freqs.begin(), freqs.end());

   // Compute the powers of each wave
   std::vector<double> powers(freqs.size());
   psd->Discretize(freqs, sp.int_method, powers);

   // Apply transfer function to the powers
   // TODO.

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
   std::visit(DirectionVisitor{k_hats}, sp.dir_params);

   // Finally, assemble the individual Wave structs
   for (std::size_t i = 0; i < sp.num_waves; i++)
   {
      const Wave w{amps[i], freqs[i], phases[i], sp.speed, k_hats[i]};
      waves.emplace_back(w);
   }
}

void SourceVisitor::operator()
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
   const CompParams &comp_conf = conf.Comp();
   const std::vector<SourceParamsVariant> &sources_conf = conf.Sources();

   // Initialize acoustic field
   AcousticField field(dim, coords, base_conf.p, base_conf.rho,
                        base_conf.U, base_conf.gamma, comp_conf.kernel);

   // Assemble vector of wave structs based on input source
   for (const SourceParamsVariant &source : sources_conf)
   {
      std::visit(SourceVisitor{field.Waves()}, source);
   }

   // Finalize the acoustic field initialization
   field.Finalize();

   return std::move(field);

}                                        

} // namespace jabber_app
