#ifdef JABBER_WITH_APP

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "app_utils.hpp"

using namespace jabber_app;
using namespace Catch::Generators;
using namespace Catch::Matchers;

namespace jabber_test
{

RandomInputXYGenerator::RandomInputXYGenerator(InputXYOption ixy_option)
: ixy_option_(ixy_option),
  x_gen_(kSize, random(0.0, 100.0e3)),
  y_gen_(kSize, random(1e-9, 1e-12)),
  ri_file_suffix_gen_(random(0, 100))
{ 
   static_cast<void>(next());
}

bool RandomInputXYGenerator::next()
{

   if (ixy_option_ == InputXYOption::Here)
   {
      x_gen_.next();
      y_gen_.next();

      InputXYParams<InputXYOption::Here> ip;
      ip.x = x_gen_.get();
      ip.y = y_gen_.get();

      ipv_ = ip;
   }
   else if (ixy_option_ == InputXYOption::FromCSV)
   {
      ri_file_suffix_gen_.next();
      InputXYParams<InputXYOption::FromCSV> ip;
      ip.file = "test_xy." + std::to_string(ri_file_suffix_gen_.get())
                   + ".csv";
      ipv_ = ip;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented InputXYOption.");
   }

   return true;
}

RandomFunctionGenerator::RandomFunctionGenerator(FunctionOption f_option)
: f_option_(f_option)
{
   static_cast<void>(next());
}

bool RandomFunctionGenerator::next()
{
   if (f_option_ == FunctionOption::PiecewiseLinear ||
       f_option_ == FunctionOption::PiecewiseLogLog)
   {
      rixy_opt_gen_.next();
      rixy_gen_.SetOption(rixy_opt_gen_.get());
      rixy_gen_.next();
      InputXYParamsVariant ixypv = rixy_gen_.get();

      if (f_option_ == FunctionOption::PiecewiseLinear)
      {
         FunctionParams<FunctionOption::PiecewiseLinear> fp;
         fp.input_xy = ixypv;
         fpv_ = fp;
      }
      else
      {
         FunctionParams<FunctionOption::PiecewiseLogLog> fp;
         fp.input_xy = ixypv;
         fpv_ = fp;
      }
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented FunctionOption.");
   }

   return true;
}

// DiscMethodParamsVariant GenerateRandomDiscMethod(DiscMethodOption d, int seed)
// {
//    DiscMethodParamsVariant dmpv;

//    if (d == DiscMethodOption::Uniform)
//    {
//       DiscMethodParams<DiscMethodOption::Uniform> dp;
//       dmpv = dp;
//    }
//    else if (d == DiscMethodOption::UniformLog)
//    {
//       DiscMethodParams<DiscMethodOption::UniformLog> dp;
//       dmpv = dp;
//    }
//    else if (d == DiscMethodOption::Random)
//    {
//       DiscMethodParams<DiscMethodOption::Random> dp;
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dmpv = dp;
//    }
//    else if (d == DiscMethodOption::RandomLog)
//    {
//       DiscMethodParams<DiscMethodOption::RandomLog> dp;
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dmpv = dp;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented DiscMethodOption.");
//    }
//    return dmpv;
// }

// DirectionParamsVariant GenerateRandomDirection(DirectionOption d, int seed)
// {
//    DirectionParamsVariant dpv;
//    if (d == DirectionOption::Constant)
//    {
//       constexpr std::size_t kDim = 3;
//       DirectionParams<DirectionOption::Constant> dp;
//       dp.direction = GenerateRandomVec<kDim>(seed++, 0, 1);
//       dpv = dp;
//    }
//    else if (d == DirectionOption::RandomXYAngle)
//    {
//       DirectionParams<DirectionOption::RandomXYAngle> dp;
//       dp.min_angle = GenerateRandomReal(seed++, -20, 20);
//       dp.max_angle = GenerateRandomReal(seed++, 20, 60);
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dpv = dp;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented DirectionOption.");
//    }
//    return dpv;
// }

// SourceParamsVariant GenerateRandomSource(SourceOption s, int seed)
// {
//    SourceParamsVariant spv;
//    if (s == SourceOption::SingleWave)
//    {
//       SourceParams<SourceOption::SingleWave> wave;
//       wave.amp = GenerateRandomReal(seed++,0.1,10.0);
//       wave.freq = GenerateRandomReal(seed++, 500.0, 1500.0);
//       wave.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
//       wave.phase = GenerateRandomReal(seed++, 10.0, 180.0);
//       wave.speed = GenerateRandomInt(seed++, 0, 1) ? 'S' : 'F';
//       spv = wave;
//    }
//    else if (s == SourceOption::WaveSpectrum)
//    {
//       constexpr int kNumWaves = 5;
//       SourceParams<SourceOption::WaveSpectrum> waves;
//       waves.amps = GenerateRandomVec<kNumWaves>(seed++, 1.0, 10.0);
//       waves.freqs = GenerateRandomVec<kNumWaves>(seed++, 500.0, 
//                                                             1500.0);
//       waves.directions.resize(kNumWaves);
//       for (std::vector<double> &w_dir : waves.directions)
//       {
//             w_dir = GenerateRandomVec<3>(seed++, 0.0, 1.0);
//       }
//       waves.phases = GenerateRandomVec<kNumWaves>(seed++, 0.0,180.0);
//       waves.speeds.resize(kNumWaves);
//       for (int w = 0; w < kNumWaves; w++)
//       {
//             waves.speeds[w] = (GenerateRandomInt(seed++, 0, 1) ? 'S' : 'F');
//       }
//       spv = waves;
//    }
//    else if (s == SourceOption::PSD)
//    {
//       SourceParams<SourceOption::PSD> source_params;
   
//       // Set fixed settings
//       source_params.dim_fac = GenerateRandomReal(seed++, 1.0, 10.0);
//       source_params.min_disc_freq = GenerateRandomReal(seed++, 1.0, 100.0);
//       source_params.max_disc_freq = GenerateRandomReal(seed++, 250e3,450e3);
//       source_params.num_waves = GenerateRandomInt(seed++, 1, 100);
//       source_params.phase_seed = GenerateRandomInt(seed++, 1, 100);

//       // Set varying settings
//       FunctionOption func_option = GenerateRandomOption<FunctionOption>(seed++);
//       source_params.input_psd = GenerateRandomFunction(func_option, seed++);

//       DiscMethodOption disc_option = GenerateRandomOption<DiscMethodOption>(seed++);
//       source_params.disc_params = GenerateRandomDiscMethod(disc_option, seed++);

//       source_params.int_method = GenerateRandomOption<Interval::Method>(seed++);

//       DirectionOption dir_option = GenerateRandomOption<DirectionOption>(seed++);
//       source_params.dir_params = GenerateRandomDirection(dir_option, seed++);

//       spv = source_params;
//    }
//    else if (s == SourceOption::WaveCSV)
//    {
//       SourceParams<SourceOption::WaveCSV> source_params;
//       source_params.file = "test_waves." + 
//                            std::to_string(GenerateRandomInt(seed++, 0, 100)) 
//                            + ".csv";
//       spv = source_params;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented SourceOption");
//    }
//    return spv;
// }

} // namespace jabber_test

#endif // JABBER_WITH_APP
