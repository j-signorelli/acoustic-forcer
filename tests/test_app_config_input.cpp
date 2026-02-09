#ifdef JABBER_WITH_APP

#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <jabber_app.hpp>

using namespace jabber_app;
using namespace Catch::Matchers;
using namespace Catch::Generators;

namespace jabber_test
{
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
               U=[{},{}]
               gamma={}
               )", kRho, kPBar, kUBar[0], kUBar[1], kGamma);
   
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
   constexpr int kNumSourceOptions = static_cast<int>(SourceOption::Size);

   // Use generator to ensure that all source types are checked!
   const int s = GENERATE_COPY(range(0, kNumSourceOptions));
   const SourceOption option = static_cast<SourceOption>(s);
   DYNAMIC_SECTION(SourceNames[s])
   {   
      if (option == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> wave; 
         wave = GenerateRandomSource<SourceOption::SingleWave>(kSeed);
         wave.speed = GENERATE('S', 'F');

         const std::string source_str = 
         std::format(R"(
                           Type="{}"
                           Amplitude={}
                           Frequency={}
                           Direction=[{},{},{}]
                           Phase={}
                           Speed='{}'
                           )", SourceNames[s], wave.amp, wave.freq, 
                              wave.direction[0], wave.direction[1], 
                              wave.direction[2], wave.phase, wave.speed);
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

         auto write_vec = 
         [&kNumWaves]<typename T>
         (const std::vector<T> &vec) -> std::string
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
         };

         source_str += "Amplitudes=" + write_vec(waves.amps) + "\n";
         source_str += "Frequencies=" + write_vec(waves.freqs) + "\n";
         source_str += "Directions=[";
         for (int w = 0; w < kNumWaves; w++)
         {
               source_str += write_vec(waves.directions[w]);
               source_str += (w+1==kNumWaves ? "]\n" : ",");
         }
         source_str += "Phases=" + write_vec(waves.phases) + "\n";
         source_str += "Speeds=" + write_vec(waves.speeds);

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
