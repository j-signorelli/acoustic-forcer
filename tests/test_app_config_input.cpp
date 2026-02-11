#ifdef JABBER_WITH_APP

#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <jabber_app.hpp>

using namespace jabber;
using namespace jabber_app;
using namespace Catch::Matchers;
using namespace Catch::Generators;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

#if __cpp_deduction_guides < 201907L
template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;
#endif

namespace jabber_test
{

// Internal helper for getting a vector as a string for TOML config files.
template<typename T>
std::string WriteVector(const std::vector<T> &vec)
{
   std::string vec_str;
   vec_str += "[";
   for (int w = 0; w < vec.size(); w++)
   {
      if constexpr (std::same_as<T, char>)
      {
         vec_str += std::format("'{}'", vec[w]);
      }
      else
      {
         vec_str += std::format("{}", vec[w]);
      }
      vec_str += (w+1==vec.size() ? "]" : ",");
   }
   return vec_str;
}

TEST_CASE("TOMLConfigInput::ParseBaseFlow", "[App][TOMLConfigInput]")
{
   int seed = 0;

   const double kRho = GenerateRandomReal(seed++, 0.1, 1.0);
   const double kPBar = GenerateRandomReal(seed++, 0.1, 2000.0);
   const std::vector<double> kUBar = GenerateRandomVec<2>(seed++, 0.0, 600.0);
   const double kGamma = GenerateRandomReal(seed++, 0.1, 1.5);

   const std::string base_flow_str = 
   std::format(R"(
               rho={}
               p={}
               U={}
               gamma={}
               )", kRho, kPBar, WriteVector(kUBar), kGamma);
   
   TOMLConfigInput config;
   config.ParseBaseFlow(base_flow_str);

   CHECK(config.BaseFlow().rho == kRho);
   CHECK(config.BaseFlow().p == kPBar);
   CHECK_THAT(config.BaseFlow().U, Equals(kUBar));
   CHECK(config.BaseFlow().gamma == kGamma);
}

TEST_CASE("TOMLConfigInput::ParseSource", "[App][TOMLConfigInput]")
{

   constexpr int kSeed = 0;
   // Use generator to ensure that all source types are checked!
   const SourceOption option = GENERATE(options<SourceOption>());
   const std::uint8_t s = static_cast<std::uint8_t>(option);
   DYNAMIC_SECTION(SourceNames[s])
   {   
      if (option == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> wave; 
         wave = GenerateRandomSource<SourceOption::SingleWave>(
                                                      kSeed, 
                                                      GENERATE('S', 'F'));

         const std::string source_str = 
         std::format(R"(
                           Type="{}"
                           Amplitude={}
                           Frequency={}
                           DirVector={}
                           Phase={}
                           Speed='{}'
                           )", SourceNames[s], wave.amp, wave.freq, 
                              WriteVector(wave.direction), wave.phase, 
                              wave.speed);
         TOMLConfigInput config;
         config.ParseSource(source_str);
         SourceParams<SourceOption::SingleWave> wave_parsed = 
               std::get<SourceParams<SourceOption::SingleWave>>(
                                                   config.Sources()[0]);
         CHECK(wave_parsed.amp == wave.amp);
         CHECK(wave_parsed.freq == wave.freq);
         CHECK_THAT(wave_parsed.direction, Equals(wave.direction));
         CHECK(wave_parsed.phase == wave.phase);
         CHECK(wave_parsed.speed == wave.speed);
      }
      else if (option == SourceOption::WaveSpectrum)
      {
         SourceParams<SourceOption::WaveSpectrum> waves;
         waves = GenerateRandomSource<SourceOption::WaveSpectrum>(kSeed);

         const int kNumWaves = waves.amps.size();
         
         std::string source_str = std::format("Type=\"{}\"\n",
                                                   SourceNames[s]);

         source_str += "Amplitudes=" + WriteVector(waves.amps) + "\n";
         source_str += "Frequencies=" + WriteVector(waves.freqs) + "\n";
         source_str += "DirVectors=[";
         for (int w = 0; w < kNumWaves; w++)
         {
               source_str += WriteVector(waves.directions[w]);
               source_str += (w+1==kNumWaves ? "]\n" : ",");
         }
         source_str += "Phases=" + WriteVector(waves.phases) + "\n";
         source_str += "Speeds=" + WriteVector(waves.speeds);

         TOMLConfigInput config;
         config.ParseSource(source_str);
         SourceParams<SourceOption::WaveSpectrum> waves_parsed = 
               std::get<SourceParams<SourceOption::WaveSpectrum>>(
                                                   config.Sources()[0]);
         
         CHECK_THAT(waves_parsed.amps, Equals(waves.amps));
         CHECK_THAT(waves_parsed.freqs, Equals(waves.freqs));
         for (int w = 0; w < kNumWaves; w++)
         {
               CHECK_THAT(waves_parsed.directions[w], 
                           Equals(waves.directions[w]));
         }
         CHECK_THAT(waves_parsed.phases, Equals(waves.phases));
         CHECK_THAT(waves_parsed.speeds, Equals(waves.speeds));
      }
      else if (option == SourceOption::DigitalPSD)
      {
         SourceParams<SourceOption::DigitalPSD> source_params;

         const PSDInputOption input_option = 
                                 GENERATE(options<PSDInputOption>());
         const InterpolationOption interp_option = 
                                 GENERATE(options<InterpolationOption>());
         const Interval::Method int_method_option = 
                                 GENERATE(options<Interval::Method>());
         const DiscMethodOption dm_option = 
                                 GENERATE(options<DiscMethodOption>());
         const DirectionOption dir_option = 
                                 GENERATE(options<DirectionOption>());

         const char speed = GENERATE('S', 'F');         
         
         source_params = GenerateRandomSource<SourceOption::DigitalPSD>(
                                    kSeed,
                                    input_option,
                                    interp_option,
                                    int_method_option,
                                    dm_option,
                                    dir_option,
                                    speed);
         std::string source_str = 
         std::format(R"(
            Type="{}"
            DimFactor={}
            Interpolation="{}"
            PhaseSeed={}
            Speed='{}'
            Discretization.Min={}
            Discretization.Max={}
            Discretization.N={}
            Discretization.Interval="{}"
            InputData.Type="{}"
            Discretization.Method.Type="{}"
            Direction.Type="{}"
            )", SourceNames[s],
            source_params.dim_fac,
            InterpolationNames[static_cast<std::uint8_t>(source_params.interp)],
            source_params.phase_seed,
            source_params.speed, source_params.min_disc_freq, 
            source_params.max_disc_freq, source_params.num_waves,
            IntervalNames[static_cast<std::uint8_t>(source_params.int_method)],
            PSDInputNames[static_cast<std::uint8_t>(input_option)],
            DiscMethodNames[static_cast<std::uint8_t>(dm_option)],
            DirectionNames[static_cast<std::uint8_t>(dir_option)]);
         
         std::visit(
         overloads
         {
         [&](const PSDInputParams<PSDInputOption::Here> input_params)
         {
            source_str = std::format(R"(
                           {}
                           InputData.Frequencies={}
                           InputData.PSDs={}
                           )", source_str, WriteVector(input_params.freqs),
                              WriteVector(input_params.psds));
         },
         [&](const PSDInputParams<PSDInputOption::FromCSV> input_params)
         {
            source_str = std::format(R"(
                           {}
                           InputData.File="{}"
                           )", source_str, input_params.file);
         }
         }, source_params.input_params);
         
         std::visit(
         overloads
         {
         [&](const DiscMethodParams<DiscMethodOption::Uniform> &disc_params)
         {
            // No params
         },
         [&](const DiscMethodParams<DiscMethodOption::UniformLog> &disc_params)
         {
            // No params
         },
         [&](const DiscMethodParams<DiscMethodOption::Random> dm_params)
         {
            source_str = std::format(R"(
                           {}
                           Discretization.Method.Seed={}
                           )", source_str, dm_params.seed);
         },
         [&](const DiscMethodParams<DiscMethodOption::RandomLog> dm_params)
         {
            source_str = std::format(R"(
                           {}
                           Discretization.Method.Seed={}
                           )", source_str, dm_params.seed);
         }
         }, source_params.disc_params);

         std::visit(
         overloads
         {
         [&](const DirectionParams<DirectionOption::Constant> dir_params)
         {
            source_str = std::format(R"(
                           {}
                           Direction.Vector={}
                           )", source_str, WriteVector(dir_params.direction));
         },
         [&](const DirectionParams<DirectionOption::RandomXYAngle> dir_params)
         {
            source_str = std::format(R"(
                           {}
                           Direction.MinAngle={}
                           Direction.MaxAngle={}
                           Direction.Seed={}
                           )", source_str, dir_params.min_angle, 
                              dir_params.max_angle, dir_params.seed);
         }
         }, source_params.dir_params);

         TOMLConfigInput config;
         config.ParseSource(source_str);
         SourceParams<SourceOption::DigitalPSD> parsed_source = 
               std::get<SourceParams<SourceOption::DigitalPSD>>(
                                                   config.Sources()[0]);

         CHECK(parsed_source.dim_fac == source_params.dim_fac);
         CHECK(parsed_source.interp == source_params.interp);
         CHECK(parsed_source.phase_seed == source_params.phase_seed);
         CHECK(parsed_source.speed == source_params.speed);
         CHECK(parsed_source.min_disc_freq == source_params.min_disc_freq);
         CHECK(parsed_source.max_disc_freq == source_params.max_disc_freq);
         CHECK(parsed_source.num_waves == source_params.num_waves);
         CHECK(parsed_source.int_method== source_params.int_method);

         std::visit(
         overloads
         {
         [&](const PSDInputParams<PSDInputOption::Here> parsed_input_params)
         {
            CHECK(input_option == PSDInputOption::Here);
            if (input_option == PSDInputOption::Here)
            {
               PSDInputParams<PSDInputOption::Here> input_params = 
                  std::get<PSDInputParams<PSDInputOption::Here>>(
                                                source_params.input_params);
               CHECK_THAT(parsed_input_params.freqs, Equals(input_params.freqs));
               CHECK_THAT(parsed_input_params.psds, Equals(input_params.psds));
            }
         },
         [&](const PSDInputParams<PSDInputOption::FromCSV> parsed_input_params)
         {
            CHECK(input_option == PSDInputOption::FromCSV);
            if (input_option == PSDInputOption::FromCSV)
            {
               PSDInputParams<PSDInputOption::FromCSV> input_params = 
                  std::get<PSDInputParams<PSDInputOption::FromCSV>>(
                                                source_params.input_params);
               CHECK(parsed_input_params.file == input_params.file);
            }
         }
         }, parsed_source.input_params);

         std::visit(
         overloads
         {
         [&](const DiscMethodParams<DiscMethodOption::Uniform> &disc_params)
         {
            // No params
            CHECK(dm_option == DiscMethodOption::Uniform);
         },
         [&](const DiscMethodParams<DiscMethodOption::UniformLog> &disc_params)
         {
            // No params
            CHECK(dm_option == DiscMethodOption::UniformLog);
         },
         [&](const DiscMethodParams<DiscMethodOption::Random> 
                                                            parsed_disc_params)
         {
            CHECK(dm_option == DiscMethodOption::Random);
            if (dm_option == DiscMethodOption::Random)
            {
               DiscMethodParams<DiscMethodOption::Random> disc_params = 
                  std::get<DiscMethodParams<DiscMethodOption::Random>>(
                                                source_params.disc_params);
               CHECK(parsed_disc_params.seed == disc_params.seed);
            }
         },
         [&](const DiscMethodParams<DiscMethodOption::RandomLog>
                                                            parsed_disc_params)
         {
            CHECK(dm_option == DiscMethodOption::RandomLog);
            if (dm_option == DiscMethodOption::RandomLog)
            {
               DiscMethodParams<DiscMethodOption::RandomLog> 
                  disc_params = 
                  std::get<DiscMethodParams<DiscMethodOption::RandomLog>>(
                                                source_params.disc_params);
               CHECK(parsed_disc_params.seed == disc_params.seed);
            }
         }
         }, parsed_source.disc_params);

         std::visit(
         overloads
         {
         [&](const DirectionParams<DirectionOption::Constant> 
                                                            parsed_dir_params)
         {
            CHECK(dir_option == DirectionOption::Constant);
            if (dir_option == DirectionOption::Constant)
            {
               DirectionParams<DirectionOption::Constant> dir_params = 
                  std::get<DirectionParams<DirectionOption::Constant>>(
                                                source_params.dir_params);
               CHECK_THAT(parsed_dir_params.direction, 
                           Equals(dir_params.direction));
            }
         },
         [&](const DirectionParams<DirectionOption::RandomXYAngle> 
                                                            parsed_disc_params)
         {
            CHECK(dir_option == DirectionOption::RandomXYAngle);
            if (dir_option == DirectionOption::RandomXYAngle)
            {
               DirectionParams<DirectionOption::RandomXYAngle> dir_params = 
                  std::get<DirectionParams<DirectionOption::RandomXYAngle>>(
                                                source_params.dir_params);
               CHECK(parsed_disc_params.min_angle == dir_params.min_angle);
               CHECK(parsed_disc_params.max_angle == dir_params.max_angle);
               CHECK(parsed_disc_params.seed == dir_params.seed);
            }
         }
         }, parsed_source.dir_params);

      }
      else if (option == SourceOption::WaveCSV)
      {
         SourceParams<SourceOption::WaveCSV> source_params;
         source_params.file = "test_waves." + 
                              std::to_string(GenerateRandomInt(0, 0, 100)) 
                              + ".csv";
         std::string source_str = std::format(R"(
                                    Type="{}"
                                    File="{}")", SourceNames[s], 
                                    source_params.file);
         TOMLConfigInput config;
         config.ParseSource(source_str);
         SourceParams<SourceOption::WaveCSV> parsed_source = 
            std::get<SourceParams<SourceOption::WaveCSV>>(config.Sources()[0]);
         CHECK(parsed_source.file == source_params.file);
      }
      else
      {
         FAIL("No unit test for source type " << SourceNames[s] << "!");
      }
   }
}

TEST_CASE("TOMLConfigInput::ParseComputation", "[App][TOMLConfigInput]")
{
   int seed = 0;
   const double kT0 = GenerateRandomReal(seed++,0.0,100.0);
   const std::string comp_str = 
      std::format(R"(
                     t0={}
                  )", kT0);

   TOMLConfigInput config;
   config.ParseComputation(comp_str);

   CHECK(config.Comp().t0 == kT0);
}

TEST_CASE("TOMLConfigInput::ParsePrecice", "[App][TOMLConfigInput]")
{
   constexpr std::string_view kPartName = "TestParticipant";
   constexpr std::string_view kConfigFile = "TestConfig.xml";
   constexpr std::string_view kFluidMesh = "TestFluidMesh";
   const std::vector<double> kMeshRegion = {-0.01, 1.01, -1.01, 1.01};

   const std::string precice_str = 
      std::format(R"(
                     ParticipantName="{}"
                     ConfigFile="{}"
                     MeshAccessRegion=[{},{},{},{}]
                     FluidMeshName="{}"
                  )", kPartName, kConfigFile, kMeshRegion[0], kMeshRegion[1],
                     kMeshRegion[2], kMeshRegion[3], kFluidMesh);
   
   TOMLConfigInput config;
   config.ParsePrecice(precice_str);
   CHECK(config.Precice()->participant_name == kPartName);
   CHECK(config.Precice()->config_file == kConfigFile);
   CHECK(config.Precice()->fluid_mesh_name == kFluidMesh);
   CHECK_THAT(config.Precice()->mesh_access_region, Equals(kMeshRegion));
}

} // jabber_test

#endif // JABBER_WITH_APP
