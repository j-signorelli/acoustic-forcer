#include "utils.hpp"

#include <jabber.hpp>
#ifdef JABBER_WITH_APP
#include <jabber_app.hpp>
#endif // JABBER_WITH_APP

using namespace jabber;
#ifdef JABBER_WITH_APP
using namespace jabber_app;
#endif // JABBER_WITH_APP

namespace jabber_test
{

#ifdef JABBER_WITH_APP

InputXYParamsVariant GenerateRandomInputXY(InputXYOption f, int seed)
{
   InputXYParamsVariant ipv;
   if (f == InputXYOption::Here)
   {
      constexpr std::size_t N = 20;
      InputXYParams<InputXYOption::Here> ip;
      ip.x = GenerateRandomVec<N>(seed++, 0, 100e3);
      ip.y = GenerateRandomVec<N>(seed++, 1e-9, 1e-12);
      ipv = ip;
   }
   else if (f == InputXYOption::FromCSV)
   {
      InputXYParams<InputXYOption::FromCSV> ip;
      ip.file = "test_xy." + 
                           std::to_string(GenerateRandomInt(seed++, 0, 100)) 
                           + ".csv";
      ipv = ip;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented InputXYOption.");
   }
   return ipv;
}

FunctionParamsVariant GenerateRandomFunction(FunctionOption f, int seed)
{
   FunctionParamsVariant fpv;
   if (f == FunctionOption::PiecewiseLinear)
   {
      FunctionParams<FunctionOption::PiecewiseLinear> fp;
      InputXYOption ixy_option = GenerateRandomOption<InputXYOption>(seed++);
      fp.input_xy = GenerateRandomInputXY(ixy_option, seed++);
      fpv = fp;
   }
   else if (f == FunctionOption::PiecewiseLogLog)
   {
      FunctionParams<FunctionOption::PiecewiseLogLog> fp;
      InputXYOption ixy_option = GenerateRandomOption<InputXYOption>(seed++);
      fp.input_xy = GenerateRandomInputXY(ixy_option, seed++);
      fpv = fp;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented FunctionOption.");
   }
   return fpv;
}

DiscMethodParamsVariant GenerateRandomDiscMethod(DiscMethodOption d, int seed)
{
   DiscMethodParamsVariant dmpv;

   if (d == DiscMethodOption::Uniform)
   {
      DiscMethodParams<DiscMethodOption::Uniform> dp;
      dmpv = dp;
   }
   else if (d == DiscMethodOption::UniformLog)
   {
      DiscMethodParams<DiscMethodOption::UniformLog> dp;
      dmpv = dp;
   }
   else if (d == DiscMethodOption::Random)
   {
      DiscMethodParams<DiscMethodOption::Random> dp;
      dp.seed = GenerateRandomInt(seed++, 0, 100);
      dmpv = dp;
   }
   else if (d == DiscMethodOption::RandomLog)
   {
      DiscMethodParams<DiscMethodOption::RandomLog> dp;
      dp.seed = GenerateRandomInt(seed++, 0, 100);
      dmpv = dp;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented DiscMethodOption.");
   }
   return dmpv;
}

DirectionParamsVariant GenerateRandomDirection(DirectionOption d, int seed)
{
   DirectionParamsVariant dpv;
   if (d == DirectionOption::Constant)
   {
      constexpr std::size_t kDim = 3;
      DirectionParams<DirectionOption::Constant> dp;
      dp.direction = GenerateRandomVec<kDim>(seed++, 0, 1);
      dpv = dp;
   }
   else if (d == DirectionOption::RandomXYAngle)
   {
      DirectionParams<DirectionOption::RandomXYAngle> dp;
      dp.min_angle = GenerateRandomReal(seed++, -20, 20);
      dp.max_angle = GenerateRandomReal(seed++, 20, 60);
      dp.seed = GenerateRandomInt(seed++, 0, 100);
      dpv = dp;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented DirectionOption.");
   }
   return dpv;
}

SourceParamsVariant GenerateRandomSource(SourceOption s, int seed)
{
   SourceParamsVariant spv;
   if (s == SourceOption::SingleWave)
   {
      SourceParams<SourceOption::SingleWave> wave;
      wave.amp = GenerateRandomReal(seed++,0.1,10.0);
      wave.freq = GenerateRandomReal(seed++, 500.0, 1500.0);
      wave.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
      wave.phase = GenerateRandomReal(seed++, 10.0, 180.0);
      wave.speed = GenerateRandomInt(seed++, 0, 1) ? 'S' : 'F';
      spv = wave;
   }
   else if (s == SourceOption::WaveSpectrum)
   {
      constexpr int kNumWaves = 5;
      SourceParams<SourceOption::WaveSpectrum> waves;
      waves.amps = GenerateRandomVec<kNumWaves>(seed++, 1.0, 10.0);
      waves.freqs = GenerateRandomVec<kNumWaves>(seed++, 500.0, 
                                                            1500.0);
      waves.directions.resize(kNumWaves);
      for (std::vector<double> &w_dir : waves.directions)
      {
            w_dir = GenerateRandomVec<3>(seed++, 0.0, 1.0);
      }
      waves.phases = GenerateRandomVec<kNumWaves>(seed++, 0.0,180.0);
      waves.speeds.resize(kNumWaves);
      for (int w = 0; w < kNumWaves; w++)
      {
            waves.speeds[w] = (GenerateRandomInt(seed++, 0, 1) ? 'S' : 'F');
      }
      spv = waves;
   }
   else if (s == SourceOption::PSD)
   {
      SourceParams<SourceOption::PSD> source_params;
   
      // Set fixed settings
      source_params.dim_fac = GenerateRandomReal(seed++, 1.0, 10.0);
      source_params.min_disc_freq = GenerateRandomReal(seed++, 1.0, 100.0);
      source_params.max_disc_freq = GenerateRandomReal(seed++, 250e3,450e3);
      source_params.num_waves = GenerateRandomInt(seed++, 1, 100);
      source_params.phase_seed = GenerateRandomInt(seed++, 1, 100);

      // Set varying settings
      FunctionOption func_option = GenerateRandomOption<FunctionOption>(seed++);
      source_params.input_psd = GenerateRandomFunction(func_option, seed++);

      DiscMethodOption disc_option = GenerateRandomOption<DiscMethodOption>(seed++);
      source_params.disc_params = GenerateRandomDiscMethod(disc_option, seed++);

      source_params.int_method = GenerateRandomOption<Interval::Method>(seed++);

      DirectionOption dir_option = GenerateRandomOption<DirectionOption>(seed++);
      source_params.dir_params = GenerateRandomDirection(dir_option, seed++);

      spv = source_params;
   }
   else if (s == SourceOption::WaveCSV)
   {
      SourceParams<SourceOption::WaveCSV> source_params;
      source_params.file = "test_waves." + 
                           std::to_string(GenerateRandomInt(seed++, 0, 100)) 
                           + ".csv";
      spv = source_params;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented SourceOption");
   }
   return spv;
}

#endif // JABBER_WITH_APP

} // namespace jabber_test
