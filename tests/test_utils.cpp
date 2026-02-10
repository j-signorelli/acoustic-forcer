#include "test_utils.hpp"

using namespace jabber;
using namespace jabber_app;

namespace jabber_test
{

#ifdef JABBER_WITH_APP

template<>
SourceParams<SourceOption::SingleWave>
GenerateRandomSource<SourceOption::SingleWave>(int seed, char speed)
{
   SourceParams<SourceOption::SingleWave> wave;
   wave.amp = GenerateRandomReal(seed++,0.1,10.0);
   wave.freq = GenerateRandomReal(seed++, 500.0, 1500.0);
   wave.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
   wave.phase = GenerateRandomReal(seed++, 10.0, 180.0);
   wave.speed = speed;
   return wave;
}

template<>
SourceParams<SourceOption::WaveSpectrum>
GenerateRandomSource<SourceOption::WaveSpectrum>(int seed)
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
   waves.phases = GenerateRandomVec<kNumWaves>(seed++,0.0,180.0);
   waves.speeds.resize(kNumWaves);
   for (int w = 0; w < kNumWaves; w++)
   {
         waves.speeds[w] = (w%2==0 ? 'S' : 'F');
   }
   return waves;
}

template<>
SourceParams<SourceOption::DigitalPSD>
GenerateRandomSource<SourceOption::DigitalPSD>(int seed,
                                    PSDInputOption input_method,
                                    InterpolationOption interp_method,
                                    Interval::Method int_method,
                                    DiscMethodOption disc_method,
                                    DirectionOption dir_method,
                                    char speed)
{

   constexpr int kNumPts = 20;

   SourceParams<SourceOption::DigitalPSD> source_params;

   if (input_method == PSDInputOption::Here)
   {
      PSDInputParams<PSDInputOption::Here> input_params;
      input_params.freqs = GenerateRandomVec<kNumPts>(seed++, 1e3, 500e3);
      input_params.psds = GenerateRandomVec<kNumPts>(seed++, 1e-9, 1e-12);
      source_params.input_params = input_params;
   }
   else if (input_method == PSDInputOption::FromCSV)
   {
      PSDInputParams<PSDInputOption::FromCSV> input_params;
      input_params.file = "sample_file" + 
                           std::to_string(GenerateRandomInt(seed++, 0,100)) 
                           + ".csv";
      source_params.input_params = input_params;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented PSDInputOption");
   }

   source_params.dim_fac = GenerateRandomReal(seed++, 1.0, 10.0);
   source_params.interp = interp_method;
   source_params.min_disc_freq = GenerateRandomReal(seed++, 1.0, 100.0);
   source_params.max_disc_freq = GenerateRandomReal(seed++, 250e3,450e3);
   source_params.num_waves = GenerateRandomInt(seed++, 1, 100);
   source_params.int_method = int_method;
   source_params.phase_speed = GenerateRandomInt(seed++, 1, 100);
   
   if (disc_method == DiscMethodOption::Random)
   {
      DiscMethodParams<DiscMethodOption::Random> disc_params;
      disc_params.seed = GenerateRandomInt(seed++, 1, 1000);
      source_params.disc_params = disc_params;
   }
   else if (disc_method == DiscMethodOption::RandomLog)
   {
      DiscMethodParams<DiscMethodOption::RandomLog> disc_params;
      disc_params.seed = GenerateRandomInt(seed++, 1, 1000);
      source_params.disc_params = disc_params;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented DiscMethodOption");
   }

   if (dir_method == DirectionOption::Constant)
   {
      DirectionParams<DirectionOption::Constant> dir_params;
      dir_params.direction = GenerateRandomVec<3>(seed++, 0.0, 1.0);
      source_params.dir_params = dir_params;
   }
   else if (dir_method == DirectionOption::RandomXYAngle)
   {
      DirectionParams<DirectionOption::RandomXYAngle> dir_params;
      dir_params.min_angle = GenerateRandomReal(seed++, -20.0, 0.0);
      dir_params.max_angle = GenerateRandomReal(seed++, 0.0, 50.0);
      dir_params.seed = GenerateRandomInt(seed++, 0, 100);
      source_params.dir_params = dir_params;
   }
   else
   {
      throw std::logic_error("Invalid or unimplemented DirectionOption");
   }
   source_params.speed = speed;

   return source_params;
}
#endif // JABBER_WITH_APP

} // namespace jabber_test
