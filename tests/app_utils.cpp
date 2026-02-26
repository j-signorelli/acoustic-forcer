#include "app_utils.hpp"

using namespace jabber_app;
using namespace Catch::Generators;

#ifdef JABBER_WITH_APP

namespace jabber_test
{

RandomInputXYGenerator::RandomInputXYGenerator
   (const std::optional<InputXYOption> option)
: option_(option),
  xy_size_(take(1,random(10,50)).get()),
  x_gen_(xy_size_, random(0.0, 100e3)),
  y_gen_(xy_size_, random(1e-9, 1e-12)),
  ri_file_suffix_gen_(random(0, 100))
{

}

bool RandomInputXYGenerator::next()
{
   InputXYOption use_option;
   if (!option_)
   {
      use_option = ro_gen_.get();
      ro_gen_.next();
   }
   else
   {
      use_option = *option_;
   }

   if (use_option == InputXYOption::Here)
   {
      x_gen_.next();
      y_gen_.next();
      
      InputXYParams<InputXYOption::Here> op;
      op.x = x_gen_.get();
      op.y = y_gen_.get();
      opv_ = op;
   }
   else if (use_option == InputXYOption::FromCSV)
   {
      ri_file_suffix_gen_.next();
      
      InputXYParams<InputXYOption::FromCSV> op;
      op.file = ri_file_suffix_gen_.get();
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented InputXYOption.");
   }
   return true;
}

// FunctionParamsVariant GenerateRandomFunction(FunctionType f, int seed)
// {
//    FunctionParamsVariant fpv;
//    if (f == FunctionType::PiecewiseLinear)
//    {
//       FunctionTypeParams<FunctionType::PiecewiseLinear> fp;
//       InputXY ixy_option = GenerateRandomOption<InputXY>(seed++);
//       fp.input_xy = GenerateRandomInputXY(ixy_option, seed++);
//       fpv = fp;
//    }
//    else if (f == FunctionType::PiecewiseLogLog)
//    {
//       FunctionTypeParams<FunctionType::PiecewiseLogLog> fp;
//       InputXY ixy_option = GenerateRandomOption<InputXY>(seed++);
//       fp.input_xy = GenerateRandomInputXY(ixy_option, seed++);
//       fpv = fp;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented FunctionType.");
//    }
//    return fpv;
// }

// DiscMethodParamsVariant GenerateRandomDiscMethod(DiscMethod d, int seed)
// {
//    DiscMethodParamsVariant dmpv;

//    if (d == DiscMethod::Uniform)
//    {
//       DiscMethodParams<DiscMethod::Uniform> dp;
//       dmpv = dp;
//    }
//    else if (d == DiscMethod::UniformLog)
//    {
//       DiscMethodParams<DiscMethod::UniformLog> dp;
//       dmpv = dp;
//    }
//    else if (d == DiscMethod::Random)
//    {
//       DiscMethodParams<DiscMethod::Random> dp;
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dmpv = dp;
//    }
//    else if (d == DiscMethod::RandomLog)
//    {
//       DiscMethodParams<DiscMethod::RandomLog> dp;
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dmpv = dp;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented DiscMethod.");
//    }
//    return dmpv;
// }

// DirectionParamsVariant GenerateRandomDirection(Direction d, int seed)
// {
//    DirectionParamsVariant dpv;
//    if (d == Direction::Constant)
//    {
//       constexpr std::size_t kDim = 3;
//       DirectionParams<Direction::Constant> dp;
//       dp.direction = GenerateRandomVec<kDim>(seed++, 0, 1);
//       dpv = dp;
//    }
//    else if (d == Direction::RandomXYAngle)
//    {
//       DirectionParams<Direction::RandomXYAngle> dp;
//       dp.min_angle = GenerateRandomReal(seed++, -20, 20);
//       dp.max_angle = GenerateRandomReal(seed++, 20, 60);
//       dp.seed = GenerateRandomInt(seed++, 0, 100);
//       dpv = dp;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented Direction.");
//    }
//    return dpv;
// }

// SourceParamsVariant GenerateRandomSource(Source s, int seed)
// {
//    SourceParamsVariant spv;
//    if (s == Source::SingleWave)
//    {
//       SourceParams<Source::SingleWave> wave;
//       wave.amp = GenerateRandomReal(seed++,0.1,10.0);
//       wave.freq = GenerateRandomReal(seed++, 500.0, 1500.0);
//       wave.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
//       wave.phase = GenerateRandomReal(seed++, 10.0, 180.0);
//       wave.speed = GenerateRandomInt(seed++, 0, 1) ? 'S' : 'F';
//       spv = wave;
//    }
//    else if (s == Source::WaveSpectrum)
//    {
//       constexpr int kNumWaves = 5;
//       SourceParams<Source::WaveSpectrum> waves;
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
//    else if (s == Source::PSD)
//    {
//       SourceParams<Source::PSD> source_params;
   
//       // Set fixed settings
//       source_params.dim_fac = GenerateRandomReal(seed++, 1.0, 10.0);
//       source_params.min_disc_freq = GenerateRandomReal(seed++, 1.0, 100.0);
//       source_params.max_disc_freq = GenerateRandomReal(seed++, 250e3,450e3);
//       source_params.num_waves = GenerateRandomInt(seed++, 1, 100);
//       source_params.phase_seed = GenerateRandomInt(seed++, 1, 100);

//       // Set varying settings
//       FunctionType func_option = GenerateRandomOption<FunctionType>(seed++);
//       source_params.input_psd = GenerateRandomFunction(func_option, seed++);

//       DiscMethod disc_option = GenerateRandomOption<DiscMethod>(seed++);
//       source_params.disc_params = GenerateRandomDiscMethod(disc_option, seed++);

//       source_params.int_method = GenerateRandomOption<Interval::Method>(seed++);

//       Direction dir_option = GenerateRandomOption<Direction>(seed++);
//       source_params.dir_params = GenerateRandomDirection(dir_option, seed++);

//       spv = source_params;
//    }
//    else if (s == Source::WaveCSV)
//    {
//       SourceParams<Source::WaveCSV> source_params;
//       source_params.file = "test_waves." + 
//                            std::to_string(GenerateRandomInt(seed++, 0, 100)) 
//                            + ".csv";
//       spv = source_params;
//    }
//    else
//    {
//       throw std::logic_error("Invalid or unimplemented Source");
//    }
//    return spv;
// }

} // namespace jabber_test

#endif // JABBER_WITH_APP
