#ifndef JABBER_TEST_APP_UTILS
#define JABBER_TEST_APP_UTILS

#ifdef JABBER_WITH_APP
#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <jabber_app.hpp>

#include <optional>

namespace jabber_test
{

/// Get the name associated with a given option enumerator.
template<OptionEnum E>
constexpr std::string_view GetName(const E &option)
{
   using namespace jabber_app;
   using namespace jabber;

   const std::size_t kIdx = static_cast<std::size_t>(option);
   if constexpr (std::same_as<E, InputXYOption>)
   {
      return kInputXYNames[kIdx];
   }
   else if constexpr (std::same_as<E, FunctionOption>)
   {
      return kFunctionNames[kIdx];
   }
   else if constexpr (std::same_as<E, Interval::Method>)
   {
      return kIntervalNames[kIdx];
   }
   else if constexpr (std::same_as<E, DiscMethodOption>)
   {
      return kDiscMethodNames[kIdx];
   }
   else if constexpr (std::same_as<E, DirectionOption>)
   {
      return kDirectionNames[kIdx];
   }
   else if constexpr (std::same_as<E, TransferOption>)
   {
      return kTransferNames[kIdx];
   }
   else if constexpr (std::same_as<E, SourceOption>)
   {
      return kSourceNames[kIdx];
   }
   else if constexpr (std::same_as<E, AcousticField::Kernel>)
   {
      return kKernelNames[kIdx];
   }
}


/// Get the variant associated with a given option enum
template<OptionEnum E>
auto GetParamsVariant()
{
   using namespace jabber_app;
   if constexpr (std::same_as<E, InputXYOption>)
   {
      return InputXYParamsVariant{};
   }
   else if constexpr (std::same_as<E, FunctionOption>)
   {
      return FunctionParamsVariant{};
   }
   else if constexpr (std::same_as<E, DiscMethodOption>)
   {
      return DiscMethodParamsVariant{};
   }
   else if constexpr (std::same_as<E, DirectionOption>)
   {
      return DirectionParamsVariant{};
   }
   else if constexpr (std::same_as<E, TransferOption>)
   {
      return TransferParamsVariant{};
   }
   else if constexpr (std::same_as<E, SourceOption>)
   {
      return SourceParamsVariant{};
   }
}

/// Alias template for a given option parameters' variant.
template<OptionEnum E>
using ParamsVariant = decltype(GetParamsVariant<E>());


/**
 * @brief Get random parameters, provided an option.
 * 
 * @details This function relies heavily on Catch2 generator wrapper creators.
 * The Catch2 seed is updated every call to them, so this function can be
 * called more than once on the same option enumerator and will yield a 
 * different output parameter. 
 * 
 */
template<OptionEnum E>
ParamsVariant<E> GetRandomParams(const E &option)
{
   using namespace jabber_app;
   using namespace Catch::Generators;

   ParamsVariant<E> opv;
   if constexpr (std::same_as<E,InputXYOption>)
   {
      if (option == InputXYOption::Here)
      {
         const int in_size = random(10, 100).get();
         InputXYParams<InputXYOption::Here> op;
         op.x = chunk(in_size, random(0.0, 100e3)).get();
         op.y = chunk(in_size, random(1e-9, 1e-12)).get();
         opv = op;
      }
      else if (option == InputXYOption::FromCSV)
      {
         const int file_suffix = random(0,100).get();
         InputXYParams<InputXYOption::FromCSV> op;
         op.file = "test_file." + std::to_string(file_suffix) 
                  + ".csv";
         opv = op;
      }
   }
   else if constexpr (std::same_as<E,FunctionOption>)
   {
      if (option == FunctionOption::PiecewiseLinear)
      {
         FunctionParams<FunctionOption::PiecewiseLinear> op;
         op.input_xy = GetRandomParams(random_option<InputXYOption>().get());
         opv = op;
      }
      else if (option == FunctionOption::PiecewiseLogLog)
      {
         FunctionParams<FunctionOption::PiecewiseLogLog> op;
         op.input_xy = GetRandomParams(random_option<InputXYOption>().get());
         opv = op;
      }
   }
   else if constexpr (std::same_as<E,DiscMethodOption>)
   {
      if (option == DiscMethodOption::Uniform)
      {
         opv = DiscMethodParams<DiscMethodOption::Uniform>{};
      }
      else if (option == DiscMethodOption::UniformLog)
      {
         opv = DiscMethodParams<DiscMethodOption::UniformLog>{};
      }
      else if (option == DiscMethodOption::Random)
      {
         DiscMethodParams<DiscMethodOption::Random> op;
         op.seed = random(0,100).get();
         opv = op;
      }
      else if (option == DiscMethodOption::RandomLog)
      {
         DiscMethodParams<DiscMethodOption::RandomLog> op;
         op.seed = random(0,100).get();
         opv = op;
      }
   }
   else if constexpr (std::same_as<E,DirectionOption>)
   {
      if (option == DirectionOption::Constant)
      {
         DirectionParams<DirectionOption::Constant> op;
         const int dim = random(1,3).get();
         op.direction = chunk(dim, random(0.0,1.0)).get();
         opv = op;
      }
      else if (option == DirectionOption::RandomXYAngle)
      {
         DirectionParams<DirectionOption::RandomXYAngle> op;
         op.min_angle = random(-20.0,20.0).get();
         op.max_angle = random(20.0,60.0).get();
         op.seed = random(0,100).get();
         opv = op;
      }
   }
   else if constexpr (std::same_as<E,TransferOption>)
   {
      // TODO
   }
   else if constexpr (std::same_as<E,SourceOption>)
   {
      if (option == SourceOption::SingleWave)
      {
         SourceParams<SourceOption::SingleWave> op;
         op.amp = random(0.1,10.0).get();
         op.freq = random(500.0,1500.0).get();
         const int dim = random(1,3).get();
         op.direction = chunk(dim, random(0.0, 1.0)).get();
         op.phase = random(10.0, 180.0).get();
         op.speed = random(0,1).get() ? 'S' : 'F';
         opv = op;
      }
      else if (option == SourceOption::WaveSpectrum)
      {
         SourceParams<SourceOption::WaveSpectrum> op;
         const std::size_t num_waves = random(1,20).get();
         for (std::size_t i = 0; i < num_waves; i++)
         {
            SourceParams<SourceOption::SingleWave> wave;
            wave = std::get<SourceParams<SourceOption::SingleWave>>
                     (GetRandomParams(SourceOption::SingleWave));
            op.amps.push_back(wave.amp);
            op.freqs.push_back(wave.freq);
            op.directions.push_back(wave.direction);
            op.phases.push_back(wave.phase);
            op.speeds.push_back(wave.speed);
         }
         opv = op;
      }
      else if (option == SourceOption::PSD)
      {
         SourceParams<SourceOption::PSD> op;
         op.input_psd = GetRandomParams(random_option<FunctionOption>().get());
         op.dim_fac = random(1.0, 10.0).get();
         op.phase_seed = random(1,100).get();
         op.speed = random(0,1).get() ? 'S' : 'F';
         op.min_disc_freq = random(1e3,10e3).get();
         op.max_disc_freq = random(100e3, 1000e3).get();
         op.num_waves = random(1,100).get();
         op.int_method = random_option<jabber::Interval::Method>().get();
         op.disc_params = GetRandomParams(random_option<DiscMethodOption>().get());
         op.dir_params = GetRandomParams(random_option<DirectionOption>().get());
         opv = op;
      }
      else if (option == SourceOption::WaveCSV)
      {
         SourceParams<SourceOption::WaveCSV> op;
         op.file = "test_waves." + std::to_string(random(0,100).get()) 
                     + ".csv";
         opv = op;
      }
   }

   return opv;
}


/// Generalized Catch 2 random parameters generator.
template<OptionEnum E>
class RandomParamsGenerator
   : public Catch::Generators::IGenerator<ParamsVariant<E>>
{
private:
   const std::optional<E> option_;
   RandomOptionGenerator<E> ro_gen_;

   ParamsVariant<E> opv_;

public:
   RandomParamsGenerator(const std::optional<E> option=std::nullopt)
   : option_(option)
   {
      static_cast<void>(next());
   }

   const ParamsVariant<E>& get() const override
   {
      return opv_;
   }

   bool next() override
   {
      E use_option;
      if (!option_)
      {
         ro_gen_.next();
         use_option = ro_gen_.get();
      }
      else
      {
         use_option = *option_;
      }

      opv_ = GetRandomParams(use_option);

      return true;
   }
};

/**
 * @brief Generator-wrapper creator for random params of 
 * \ref OptionEnum. Use by \c random_params<E>(E::V) or with varying param
 * type by \c random_params<E>().
 */
template<OptionEnum E>
Catch::Generators::GeneratorWrapper<ParamsVariant<E>> random_params
   (std::optional<E> V=std::nullopt)
{
   return Catch::Generators::GeneratorWrapper<ParamsVariant<E>>(
            Catch::Detail::make_unique<RandomParamsGenerator<E>>(V));
}

/// Simple type trait for std::vector.
template<typename T>
struct is_std_vector : std::false_type {};

template<typename... VArgs>
struct is_std_vector<std::vector<VArgs...>> : std::true_type {};

/// Helper for writing to a TOML value.
template<typename T>
std::string TOMLWriteValue(const T &val)
{
   std::string out_str;

   if constexpr (is_std_vector<T>::value)
   {
      out_str += "[";
      for (int w = 0; w < val.size(); w++)
      {
         out_str += TOMLWriteValue(val[w]);
         out_str += (w+1==val.size() ? "]" : ",");
      }
   }
   else if constexpr (std::same_as<T, char> || std::same_as<T, std::string> || 
                        std::same_as<T, std::string_view>)
   {
      out_str += std::format("'{}'", val);
   }
   else
   {
      out_str += std::format("{}", val);
   }
   return out_str;
}

/**
 * @brief Write the given option parameter variant to TOML-format.
 */
template<OptionEnum E>
std::string TOMLWriteParams
   (const ParamsVariant<E> &opv, bool inline_table=false)
{
   using namespace jabber_app;

   std::map<std::string, std::string> out_params;

   std::visit(
   [=,&out_params]<template<E> typename Params, E V>(const Params<V> &op)
   {
      out_params.emplace("Type", TOMLWriteValue(GetName(V)));

      if constexpr (std::same_as<E, InputXYOption>)
      {
         if constexpr (V == InputXYOption::Here)
         {
            out_params.emplace("X", TOMLWriteValue(op.x));
            out_params.emplace("Y", TOMLWriteValue(op.y));
         }
         else if constexpr (V == InputXYOption::FromCSV)
         {
            out_params.emplace("File",  TOMLWriteValue(op.file));
         }
      }
      else if constexpr (std::same_as<E, FunctionOption>)
      {
         if constexpr (V == FunctionOption::PiecewiseLinear ||
                        V == FunctionOption::PiecewiseLogLog)
         {
            out_params.emplace
               ("Data", TOMLWriteParams<InputXYOption>(op.input_xy, true));
         }
      }
      else if constexpr (std::same_as<E, DiscMethodOption>)
      {
         if constexpr (V == DiscMethodOption::Random || 
                        V == DiscMethodOption::RandomLog)
         {
            out_params.emplace("Seed",  TOMLWriteValue(op.seed));
         }
      }
      else if constexpr (std::same_as<E, DirectionOption>)
      {
         if constexpr (V == DirectionOption::Constant)
         {
            out_params.emplace("Vector", TOMLWriteValue(op.direction));
         }
         else if constexpr (V == DirectionOption::RandomXYAngle)
         {
            out_params.emplace("MinAngle", TOMLWriteValue(op.min_angle));
            out_params.emplace("MaxAngle", TOMLWriteValue(op.max_angle));
            out_params.emplace("Seed", TOMLWriteValue(op.seed));
         }
      }
      else if constexpr (std::same_as<E, TransferOption>)
      {
         // TODO.
         // if constexpr (V == TransferOption::Input)
         // {
         //    out_params.emplace("")
         // }
      }
      else if constexpr (std::same_as<E, SourceOption>)
      {
         if constexpr (V == SourceOption::SingleWave)
         {
            out_params.emplace("Amplitude", TOMLWriteValue(op.amp));
            out_params.emplace("Frequency", TOMLWriteValue(op.freq));
            out_params.emplace("DirVector", TOMLWriteValue(op.direction));
            out_params.emplace("Phase", TOMLWriteValue(op.phase));
            out_params.emplace("Speed", TOMLWriteValue(op.speed));
         }
         else if constexpr (V == SourceOption::WaveSpectrum)
         {
            out_params.emplace("Amplitudes", TOMLWriteValue(op.amps));
            out_params.emplace("Frequencies", TOMLWriteValue(op.freqs));
            out_params.emplace("DirVectors", TOMLWriteValue(op.directions));
            out_params.emplace("Phases", TOMLWriteValue(op.phases));
            out_params.emplace("Speeds", TOMLWriteValue(op.speeds));
         }
         else if constexpr (V == SourceOption::PSD)
         {
            out_params.emplace("InputPSD", 
                        TOMLWriteParams<FunctionOption>(op.input_psd, true));
            out_params.emplace("ScaleFactor", TOMLWriteValue(op.dim_fac));
            out_params.emplace("PhaseSeed", TOMLWriteValue(op.phase_seed));
            out_params.emplace("Speed", TOMLWriteValue(op.speed));
            out_params.emplace("Discretization.Min", 
                                 TOMLWriteValue(op.min_disc_freq));
            out_params.emplace("Discretization.Max", 
                                 TOMLWriteValue(op.max_disc_freq));
            out_params.emplace("Discretization.N", 
                                 TOMLWriteValue(op.num_waves));
            out_params.emplace("Discretization.Interval", 
                                 TOMLWriteValue(GetName(op.int_method)));
            out_params.emplace("Discretization.Method", 
                        TOMLWriteParams<DiscMethodOption>(op.disc_params, true));
            out_params.emplace("Direction", 
                        TOMLWriteParams<DirectionOption>(op.dir_params, true));
         }
         else if constexpr (V == SourceOption::WaveCSV)
         {
            out_params.emplace("File", TOMLWriteValue(op.file));
         }
      }

   }, opv);


   std::string out_str;
   if (inline_table)
   {
      out_str += "{";
   }
   for (auto iter = out_params.begin(); iter != out_params.end(); iter++)
   {
      const auto &kv = *iter;

      out_str += kv.first + "=" + kv.second;
      if (inline_table && std::next(iter) != out_params.end())
      {
         out_str += ",";
      }
      else if (!inline_table)
      {
         out_str += "\n";
      }
   }
   if (inline_table)
   {
      out_str += "}";
   }

   return out_str;
}


template<OptionEnum E>
void TestParamsEqual
   (const ParamsVariant<E> &opv1, const ParamsVariant<E> &opv2)
{
   using namespace jabber_app;
   using namespace Catch::Matchers;

   std::visit(
   []<template<E> typename Params, E V1, E V2>
      (const Params<V1> &op1, const Params<V2> &op2)
   {
      // First make sure these are indeed the same types
      if constexpr (V1 != V2)
      {
         CAPTURE(GetName(V1), GetName(V2));
         FAIL("Non-matching types!");
      }
      else // else the types match!
      {
         if constexpr (std::same_as<E,InputXYOption>)
         {
            if constexpr (V1 == InputXYOption::Here)
            {
               CHECK_THAT(op1.x, Equals(op2.x));
               CHECK_THAT(op1.y, Equals(op2.y));
            }
            else if constexpr (V1 == InputXYOption::FromCSV)
            {
               CHECK(op1.file == op2.file);
            }
         }
         else if constexpr (std::same_as<E,FunctionOption>)
         {
            if constexpr (V1 == FunctionOption::PiecewiseLinear ||
                           V1 == FunctionOption::PiecewiseLogLog)
            {
               TestParamsEqual<InputXYOption>(op1.input_xy, op2.input_xy);
            }
         }
         else if constexpr (std::same_as<E, DiscMethodOption>)
         {
            if constexpr (V1 == DiscMethodOption::Random || 
                           V1 == DiscMethodOption::RandomLog)
            {
               CHECK(op1.seed == op2.seed);
            }
         }
         else if constexpr (std::same_as<E, DirectionOption>)
         {
            if constexpr (V1 == DirectionOption::Constant)
            {
               CHECK_THAT(op1.direction, Equals(op2.direction));
            }
            else if constexpr (V1 == DirectionOption::RandomXYAngle)
            {
               CHECK(op1.min_angle == op2.min_angle);
               CHECK(op1.max_angle == op2.max_angle);
               CHECK(op1.seed == op2.seed);
            }
         }
         else if constexpr (std::same_as<E,TransferOption>)
         {
            // TODO
         }
         else if constexpr (std::same_as<E, SourceOption>)
         {
            if constexpr (V1 == SourceOption::SingleWave)
            {
               CHECK(op1.amp == op2.amp);
               CHECK(op1.freq == op2.freq);
               CHECK_THAT(op1.direction, Equals(op2.direction));
               CHECK(op1.phase == op2.phase);
               CHECK(op1.speed == op2.speed);
            }
            else if constexpr (V1 == SourceOption::WaveSpectrum)
            {
               CHECK_THAT(op1.amps, Equals(op2.amps));
               CHECK_THAT(op1.freqs, Equals(op2.freqs));
               CHECK_THAT(op1.phases, Equals(op2.phases));
               CHECK_THAT(op2.speeds, Equals(op2.speeds));
               REQUIRE(op1.directions.size() == op2.directions.size());
               for (std::size_t i = 0; i < op1.directions.size(); i++)
               {
                  CHECK_THAT(op1.directions[i], Equals(op2.directions[i]));
               }
            }
            else if constexpr (V1 == SourceOption::PSD)
            {
               CHECK(op1.dim_fac == op2.dim_fac);
               CHECK(op1.min_disc_freq == op2.min_disc_freq);
               CHECK(op1.max_disc_freq == op2.max_disc_freq);
               CHECK(op1.num_waves == op2.num_waves);
               CHECK(op1.int_method == op2.int_method);
               CHECK(op1.phase_seed == op2.phase_seed);
               CHECK(op1.speed == op2.speed);
               TestParamsEqual<FunctionOption>(op1.input_psd, op2.input_psd);
               TestParamsEqual<DiscMethodOption>(op1.disc_params, 
                                                   op2.disc_params);
               TestParamsEqual<DirectionOption>(op1.dir_params, 
                                                op2.dir_params);                                                   

            }
            else if constexpr (V1 == SourceOption::WaveSpectrum)
            {
               CHECK(op1.file == op2.file);
            }
         }
      }
   }, opv1, opv2);
}

} // namespace jabber_test

#endif // JABBER_WITH_APP

#endif // JABBER_TEST_APP_UTILS