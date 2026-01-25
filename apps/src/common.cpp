#include "common.hpp"

#include <cmath>

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
         bool slow = (params_wave.speed == SpeedOption::Slow ? true : false);
         std::vector<double> k_hat(dim, 0.0);
         Normalize(params_wave.direction, k_hat);
         all_waves.emplace_back(params_wave.amp, params_wave.freq, 
                              params_wave.phase*M_PI/180.0, slow, k_hat);
      },
      [&](const SourceParams<SourceOption::WaveSpectrum> &params_waves)
      {
         for (int i = 0; i < params_waves.amps.size(); i++)
         {
            bool slow = (params_waves.speeds[i] == 
                                          SpeedOption::Slow ? true : false);
            std::vector<double> k_hat(dim, 0.0);
            Normalize(params_waves.directions[i], k_hat);
            all_waves.emplace_back(params_waves.amps[i], params_waves.freqs[i],
                                    params_waves.phases[i]*M_PI/180.0, 
                                    slow, k_hat);
         }
      }
      }, source);
   }

   // Apply transfer function
   // TODO


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
