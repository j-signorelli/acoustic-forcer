#ifdef JABBER_WITH_APP

#include "app_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <jabber_app.hpp>

using namespace jabber;
using namespace jabber_app;
using namespace Catch::Matchers;
using namespace Catch::Generators;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

namespace jabber_test
{

/// Helper for getting a vector as a string for TOML config files.
template<typename T>
std::string TOMLWriteVector(const std::vector<T> &vec)
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
   const double kRho = GENERATE(take(1,random(0.1, 1.0)));
   const double kPBar = GENERATE(take(1,random(0.1, 2000.0)));
   const std::vector<double> kUBar = 
                              GENERATE(take(1,chunk(2,random(0.0, 600.0))));
   const double kGamma = GENERATE(take(1,random(0.1, 1.5)));

   const std::string base_flow_str = 
   std::format("rho={}\n"
               "p={}\n"
               "U={}\n"
               "gamma={}", kRho, kPBar, TOMLWriteVector(kUBar), kGamma);
   
   BaseFlowParams bf_params;
   TOMLConfigInput::ParseBaseFlow(base_flow_str, bf_params);

   CHECK(bf_params.rho == kRho);
   CHECK(bf_params.p == kPBar);
   CHECK_THAT(bf_params.U, Equals(kUBar));
   CHECK(bf_params.gamma == kGamma);
}

TEST_CASE("TOMLConfigInput::ParseComputation", "[App][TOMLConfigInput]")
{
   int seed = 0;

   const double kT0 = GENERATE(take(1,random(0.0,100.0)));

   const AcousticField::Kernel kKernel = 
                                 GENERATE(options<AcousticField::Kernel>());
   const std::string_view kKernelName = 
      OptionNames<AcousticField::Kernel>[static_cast<std::size_t>(kKernel)];

   const std::string comp_str = 
      std::format(R"(
                     t0={}
                     Kernel='{}'
                  )", kT0, 
                  kKernelName);

   TOMLConfigInput config;
   config.ParseComputation(comp_str);

   CHECK(config.Comp().t0 == kT0);
   CHECK(config.Comp().kernel == kKernel);
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

// ----------------------------------------------------------------------------
// Now all options parsing:

/// Base TOML writer visitor for options.
template<OptionEnum E>
struct TOMLWriterVisitor;

/// InputXY TOML writer visitor.
template<>
struct TOMLWriterVisitor<InputXY>
{
   template<InputXY V>
   std::string operator() (const OptionParams<V> &op)
   {
      std::string out_str = std::format("Type=\"{}\",\n", 
                                          OptionName<V>);

      if constexpr (V == InputXY::Here)
      {
         out_str += std::format( "X={}\n"
                                 "Y={}\n",
                                 TOMLWriteVector(op.x), 
                                 TOMLWriteVector(op.y));
      }
      else if constexpr (V == InputXY::FromCSV)
      {
         out_str += std::format("File=\"{}\"\n",op.file);
      }
      else
      {
         static_assert(false, "Missing params implementation.");
      }
      return out_str;
   }
};

/// FunctionType TOML writer visitor.
template<>
struct TOMLWriterVisitor<FunctionType>
{
   template<FunctionType V>
   std::string operator() (const OptionParams<V> &fp)
   {
      std::string out_str = std::format("Type=\"{}\"\n", 
                                          OptionName<V>);

      if constexpr (V == FunctionOption::PiecewiseLinear ||
                     V == FunctionOption::PiecewiseLogLog)
      {
         out_str += std::format( "Data={{{}}}\n",
                                 std::visit(TOMLInputXYWriterVisitor{}, 
                                             fp.input_xy));
      }
      else
      {
         static_assert(false, "Missing params implementation.");
      }

      return out_str;
   }
   
};

/// Get the option, given the variant.
template<OptionEnum E>
constexpr E GetOption(const OptionParams<E> &var)
{
   // Get the type
   return std::visit(
            [&]<E V>(const Params<V> &val)
            {
               return V;
            }, var);
}

/// Base option test visitor.
template<OptionEnum E>
struct TestVisitor;

/// InputXY test visitor.
template<>
struct TestVisitor<InputXY>
{
   const OptionParamsVar<InputXY> &opv;

   template<InputXY V>
   void operator() (const OptionParams<V> &op_test) const
   {
      REQUIRE(GetOption(opv) == V);
      const OptionParams<V> &op = 
         std::get<OptionParams<V>>(opv);

      if constexpr (V == InputXY::Here)
      {
         CHECK_THAT(op.x, Catch::Matchers::Equals(op_test.x));
         CHECK_THAT(op.y, Catch::Matchers::Equals(op_test.y));
      }
      else if constexpr (V == InputXY::FromCSV)
      {
         CHECK(op.file == op_test.file);
      }
      else
      {
         static_assert(false, "Missing params implementation.");
      }
   }
};

/// FunctionType test visitor.
template<>
struct TestVisitor<FunctionType>
{
   const OptionParamsVar<FunctionType> &opv;

   template<FunctionType V>
   void operator() (const OptionParams<V> &op_test) const
   {
      REQUIRE(GetOption(opv) == V);
      const OptionParams<V> &op = 
         std::get<OptionParams<V>>(opv);

      if constexpr (V == FunctionType::PiecewiseLinear || 
                     V == FunctionType::PiecewiseLogLog)
      {
         std::visit(TestVisitor<InputXY>{op.input_xy}, op_test.input_xy);
      }
   }
};

TEMPLATE_TEST_CASE("TOMLConfigInput::ParseOption", "[App][TOMLConfigInput]",
   InputXY, FunctionType)
{  
   const TestType kOption = GENERATE(options<TestType>());
   OptionParamsVar<TestType> opv = 
      GENERATE_REF(take(1, random_params(kOption)));

   std::string in_str = std::visit(TOMLWriterVisitor<TestType>{}, opv);

   OptionParamsVar<TestType> opv_parsed;
   TOMLConfigInput::ParseOption(in_str, opv_parsed);

   std::visit(TestVisitor<TestType>{opv}, opv_parsed);
}

// TEST_CASE("TOMLConfigInput::ParseSource", "[App][TOMLConfigInput]")
// {

//    constexpr int kSeed = 0;
//    // Use generator to ensure that all source types are checked!
//    const SourceOption option = GENERATE(options<SourceOption>());
//    const std::uint8_t s = static_cast<std::uint8_t>(option);
//    DYNAMIC_SECTION(SourceNames[s])
//    {   
//       SourceParamsVariant sp_var = GenerateRandomSource(option, kSeed);
//       std::visit(
//       overloads
//       {
//       [&](SourceParams<SourceOption::SingleWave> &wave)
//       {
//          wave.speed = GENERATE('S', 'F');

//          const std::string source_str = 
//          std::format(R"(
//                            Type="{}"
//                            Amplitude={}
//                            Frequency={}
//                            DirVector={}
//                            Phase={}
//                            Speed='{}'
//                            )", SourceNames[s], wave.amp, wave.freq, 
//                               WriteVector(wave.direction), wave.phase, 
//                               wave.speed);
//          TOMLConfigInput config;
//          config.ParseSource(source_str);
//          SourceParams<SourceOption::SingleWave> wave_parsed = 
//                std::get<SourceParams<SourceOption::SingleWave>>(
//                                                    config.Sources()[0]);
//          CHECK(wave_parsed.amp == wave.amp);
//          CHECK(wave_parsed.freq == wave.freq);
//          CHECK_THAT(wave_parsed.direction, Equals(wave.direction));
//          CHECK(wave_parsed.phase == wave.phase);
//          CHECK(wave_parsed.speed == wave.speed);
//       },
//       [&](SourceParams<SourceOption::WaveSpectrum> &waves)
//       {
//          const int kNumWaves = waves.amps.size();
         
//          std::string source_str = std::format("Type=\"{}\"\n",
//                                                    SourceNames[s]);

//          source_str += "Amplitudes=" + WriteVector(waves.amps) + "\n";
//          source_str += "Frequencies=" + WriteVector(waves.freqs) + "\n";
//          source_str += "DirVectors=[";
//          for (int w = 0; w < kNumWaves; w++)
//          {
//                source_str += WriteVector(waves.directions[w]);
//                source_str += (w+1==kNumWaves ? "]\n" : ",");
//          }
//          source_str += "Phases=" + WriteVector(waves.phases) + "\n";
//          source_str += "Speeds=" + WriteVector(waves.speeds);

//          TOMLConfigInput config;
//          config.ParseSource(source_str);
//          SourceParams<SourceOption::WaveSpectrum> waves_parsed = 
//                std::get<SourceParams<SourceOption::WaveSpectrum>>(
//                                                    config.Sources()[0]);
         
//          CHECK_THAT(waves_parsed.amps, Equals(waves.amps));
//          CHECK_THAT(waves_parsed.freqs, Equals(waves.freqs));
//          for (int w = 0; w < kNumWaves; w++)
//          {
//                CHECK_THAT(waves_parsed.directions[w], 
//                            Equals(waves.directions[w]));
//          }
//          CHECK_THAT(waves_parsed.phases, Equals(waves.phases));
//          CHECK_THAT(waves_parsed.speeds, Equals(waves.speeds));
//       },
//       [&](SourceParams<SourceOption::PSD> &source_params)
//       {
//          // Test all combinations of variable settings
//          const FunctionOption func_option = 
//                                        GENERATE(options<FunctionOption>());
//          source_params.input_psd = GenerateRandomFunction(
//                                        func_option,
//                                        kSeed);
//          std::visit(
//          [&]<FunctionOption F>(FunctionParams<F> &fp)
//          {
//             if constexpr (F == FunctionOption::PiecewiseLinear ||
//                           F == FunctionOption::PiecewiseLogLog)
//             {
//                fp.input_xy = GenerateRandomInputXY(
//                                  GENERATE(options<InputXYOption>()), 
//                                  kSeed);
//             }
//          }, source_params.input_psd);

//          const DiscMethodOption disc_method_option = 
//                                        GENERATE(options<DiscMethodOption>());
//          source_params.disc_params = GenerateRandomDiscMethod(
//                                        disc_method_option,
//                                        kSeed);
//          source_params.int_method = GENERATE(options<Interval::Method>());

//          const DirectionOption dir_option = 
//                                        GENERATE(options<DirectionOption>());
//          source_params.dir_params = GenerateRandomDirection(
//                                        dir_option,
//                                        kSeed);
//          source_params.speed = GENERATE('S', 'F');         
         
//          std::string source_str = 
//          std::format(R"(
//             Type="{}"
//             ScaleFactor={}
//             PhaseSeed={}
//             Speed='{}'
//             Discretization.Min={}
//             Discretization.Max={}
//             Discretization.N={}
//             Discretization.Interval="{}"

//             InputPSD.Type="{}"
//             Discretization.Method.Type="{}"
//             Direction.Type="{}"
//             )", SourceNames[s],
//             source_params.dim_fac,
//             source_params.phase_seed,
//             source_params.speed, source_params.min_disc_freq, 
//             source_params.max_disc_freq, source_params.num_waves,
//             IntervalNames[static_cast<std::uint8_t>(source_params.int_method)],
//             FunctionNames[static_cast<std::uint8_t>(func_option)],
//             DiscMethodNames[static_cast<std::uint8_t>(disc_method_option)],
//             DirectionNames[static_cast<std::uint8_t>(dir_option)]);
         
//          std::visit(
//          [&]<FunctionOption F>(const FunctionParams<F> &fp)
//          {
//             if constexpr (F == FunctionOption::PiecewiseLinear ||
//                           F == FunctionOption::PiecewiseLogLog)
//             {
//                std::visit(
//                overloads
//                {
//                [&](const InputXYParams<InputXYOption::Here> &ip)
//                {
//                   source_str = std::format(R"(
//                                     {}
//                                     InputPSD.Data.Type="{}"
//                                     InputPSD.Data.X={}
//                                     InputPSD.Data.Y={}
//                                     )", source_str,
//                                     InputXYNames[static_cast<std::size_t>(
//                                                    InputXYOption::Here)],
//                                     WriteVector(ip.x), WriteVector(ip.y));
//                },
//                [&](const InputXYParams<InputXYOption::FromCSV> &ip)
//                {
//                   source_str = std::format(R"(
//                                     {}
//                                     InputPSD.Data.Type="{}"
//                                     InputPSD.Data.File="{}"
//                                     )", 
//                                     source_str,
//                                     InputXYNames[static_cast<std::size_t>(
//                                                    InputXYOption::FromCSV)], ip.file);
//                }
//                }, fp.input_xy);
//             }
//          }, source_params.input_psd);

         
//          std::visit(
//          [&source_str]<DiscMethodOption D>(const DiscMethodParams<D> &dp)
//          {
//             if constexpr (D == DiscMethodOption::Random ||
//                            D == DiscMethodOption::RandomLog)
//             {
//                source_str = std::format(R"(
//                               {}
//                               Discretization.Method.Seed={}
//                               )", source_str, dp.seed);
//             }
//          }, source_params.disc_params);

//          std::visit(
//          overloads
//          {
//          [&source_str](const DirectionParams<DirectionOption::Constant> &dp)
//          {
//             source_str = std::format(R"(
//                            {}
//                            Direction.Vector={}
//                            )", source_str, WriteVector(dp.direction));
//          },
//          [&source_str](const DirectionParams<DirectionOption::RandomXYAngle>
//                                                                         &dp)
//          {
//             source_str = std::format(R"(
//                            {}
//                            Direction.MinAngle={}
//                            Direction.MaxAngle={}
//                            Direction.Seed={}
//                            )", source_str, dp.min_angle, 
//                               dp.max_angle, dp.seed);
//          }
//          }, source_params.dir_params);

//          TOMLConfigInput config;
//          config.ParseSource(source_str);
//          SourceParams<SourceOption::PSD> parsed_source = 
//                std::get<SourceParams<SourceOption::PSD>>(
//                                                    config.Sources()[0]);

//          CHECK(parsed_source.dim_fac == source_params.dim_fac);
//          CHECK(parsed_source.phase_seed == source_params.phase_seed);
//          CHECK(parsed_source.speed == source_params.speed);
//          CHECK(parsed_source.min_disc_freq == source_params.min_disc_freq);
//          CHECK(parsed_source.max_disc_freq == source_params.max_disc_freq);
//          CHECK(parsed_source.num_waves == source_params.num_waves);
//          CHECK(parsed_source.int_method== source_params.int_method);

//          std::visit(
//          [&]<FunctionOption F>(const FunctionParams<F> &parsed_fp)
//          {
//             CHECK(func_option == F);
//             const FunctionParams<F> &source_ip = 
//                std::get<FunctionParams<F>>(source_params.input_psd);
//             if constexpr (F == FunctionOption::PiecewiseLinear || 
//                            F == FunctionOption::PiecewiseLogLog)
//             {
               
//                std::visit(
//                overloads
//                {
//                [&](const InputXYParams<InputXYOption::Here> &parsed_ip)
//                {
//                   InputXYParams<InputXYOption::Here> input_params = 
//                      std::get<InputXYParams<InputXYOption::Here>>(
//                                                          source_ip.input_xy);
//                   CHECK_THAT(parsed_ip.x, Equals(input_params.x));
//                   CHECK_THAT(parsed_ip.y, Equals(input_params.y));
//                },
//                [&](const InputXYParams<InputXYOption::FromCSV> &parsed_ip)
//                {
//                   InputXYParams<InputXYOption::FromCSV> input_params = 
//                      std::get<InputXYParams<InputXYOption::FromCSV>>(
//                                                          source_ip.input_xy);
//                   CHECK(parsed_ip.file == input_params.file);
//                }
//                }, parsed_fp.input_xy);
//             }
//          }, parsed_source.input_psd);

//          std::visit(
//          [&disc_method_option, &source_params]
//             <DiscMethodOption D>(const DiscMethodParams<D> &parsed_dp)
//          {
//             CHECK(disc_method_option == D);
//             if constexpr (D == DiscMethodOption::Random || 
//                            D == DiscMethodOption::RandomLog)
//             {
//                DiscMethodParams<D> disc_params = 
//                      std::get<DiscMethodParams<D>>(source_params.disc_params);
//                CHECK(parsed_dp.seed == disc_params.seed);
//             } 
//          }, parsed_source.disc_params);

//          std::visit(
//          overloads
//          {
//          [&](const DirectionParams<DirectionOption::Constant> &parsed_dp)
//          {
//             CHECK(dir_option == DirectionOption::Constant);
//             if (dir_option == DirectionOption::Constant)
//             {
//                DirectionParams<DirectionOption::Constant> dir_params = 
//                   std::get<DirectionParams<DirectionOption::Constant>>(
//                                                 source_params.dir_params);
//                CHECK_THAT(parsed_dp.direction, 
//                            Equals(dir_params.direction));
//             }
//          },
//          [&](const DirectionParams<DirectionOption::RandomXYAngle> &parsed_dp)
//          {
//             CHECK(dir_option == DirectionOption::RandomXYAngle);
//             if (dir_option == DirectionOption::RandomXYAngle)
//             {
//                DirectionParams<DirectionOption::RandomXYAngle> dir_params = 
//                   std::get<DirectionParams<DirectionOption::RandomXYAngle>>(
//                                                 source_params.dir_params);
//                CHECK(parsed_dp.min_angle == dir_params.min_angle);
//                CHECK(parsed_dp.max_angle == dir_params.max_angle);
//                CHECK(parsed_dp.seed == dir_params.seed);
//             }
//          }
//          }, parsed_source.dir_params);

//       },
//       [&](SourceParams<SourceOption::WaveCSV> &source_params)
//       {
//          source_params.file = "test_waves." + 
//                               std::to_string(GenerateRandomInt(kSeed, 0, 100)) 
//                               + ".csv";
//          std::string source_str = std::format(R"(
//                                     Type="{}"
//                                     File="{}")", SourceNames[s], 
//                                     source_params.file);
//          TOMLConfigInput config;
//          config.ParseSource(source_str);
//          SourceParams<SourceOption::WaveCSV> parsed_source = 
//             std::get<SourceParams<SourceOption::WaveCSV>>(config.Sources()[0]);
//          CHECK(parsed_source.file == source_params.file);
//       }
//       }, sp_var);
//    }
// }

} // jabber_test

#endif // JABBER_WITH_APP
