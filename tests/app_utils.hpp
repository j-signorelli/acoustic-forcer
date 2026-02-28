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

/**
 * @brief Generator for a random \ref jabber_app::InputXYParamsVariant.
 * 
 * @details If option is provided at construction, params type will stay 
 * same throughout object life. Otherwise, params type is randomized 
 * with every call to \ref next().
 */
class RandomInputXYGenerator 
   : public Catch::Generators::IGenerator<jabber_app::InputXYParamsVariant>
{
private:
   const std::optional<jabber_app::InputXYOption> option_;
   RandomOptionGenerator<jabber_app::InputXYOption> ro_gen_;

   jabber_app::InputXYParamsVariant opv_;

   // For InputXYOption::Here
   const std::size_t xy_size_;
   Catch::Generators::ChunkGenerator<double> x_gen_, y_gen_;

   // For InputXYOption::FromCSV
   Catch::Generators::GeneratorWrapper<int> ri_file_suffix_gen_;

public:
   RandomInputXYGenerator
      (const std::optional<jabber_app::InputXYOption> option=std::nullopt);

   jabber_app::InputXYParamsVariant const& get() const override
   {
      return opv_;
   }

   bool next() override;
};

/**
 * @brief Generator for a random \ref jabber_app::FunctionParamsVariant.
 * 
 * @details If option is provided at construction, params type will stay 
 * same throughout object life. Otherwise, params type is randomized 
 * with every call to \ref next().
 */
class RandomFunctionGenerator 
   : public Catch::Generators::IGenerator<jabber_app::FunctionParamsVariant>
{
private:
   const std::optional<jabber_app::FunctionOption> option_;
   RandomOptionGenerator<jabber_app::FunctionOption> ro_gen_;

   jabber_app::FunctionParamsVariant opv_;

   // For FunctionOption::PiecewiseLinear and FunctionOption::PiecewiseLogLog
   RandomOptionGenerator<jabber_app::InputXYOption> rixy_opt_gen_;
   RandomInputXYGenerator rixy_gen_;

public:
   RandomFunctionGenerator
      (const std::optional<jabber_app::FunctionOption> option=std::nullopt)
   : option_(option)
   {
      static_cast<void>(next());
   }
   jabber_app::FunctionParamsVariant const& get() const override
   {
      return opv_;
   }

   bool next() override;
};


/**
 * @brief Generator-wrapper creator for random params of 
 * \ref OptionEnum. Use by \c random_params<E>(E::V) or with varying param
 * type by \c random_params<E>().
 */
template<OptionEnum E>
constexpr auto random_params(std::optional<E> V=std::nullopt)
{
   using namespace jabber_app;
   using RandomGeneratorType = 
   decltype([]()
   {
      if constexpr (std::same_as<E, InputXYOption>)
      {
         return RandomInputXYGenerator();
      }
      else if constexpr (std::same_as<E, FunctionOption>)
      {
         return RandomFunctionGenerator();
      }
      else if constexpr (std::same_as<E, DiscMethodOption>)
      {
         return RandomDiscMethodGenerator();
      }
      else if constexpr (std::same_as<E, DirectionOption>)
      {
         return RandomDirectionGenerator();
      }
      else if constexpr (std::same_as<E, TransferOption>)
      {
         return RandomTransferGenerator();
      }
      else if constexpr (std::same_as<E, SourceOption>)
      {
         return RandomSourceGenerator();
      }

   }());

   using ReturnType = 
   decltype([]()
   {
      RandomGeneratorType t;
      return t.get();
   }());

   return Catch::Generators::GeneratorWrapper<ReturnType>(
            Catch::Detail::make_unique<RandomGeneratorType>(V));
}

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

/// Simple type trait for std vectors
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
   else if constexpr (std::same_as<T, char> || std::same_as<T, std::string>)
   {
      out_str += std::format("'{}'", val);
   }
   else
   {
      out_str += std::format("{}", val);
   }
   return out_str;
}

template<OptionEnum E, template<E> typename Params, E... Vs>
std::string TOMLWriteParams
   (const std::variant<Params<Vs>...> &opv, bool inline_table=false)
{
   using namespace jabber_app;

   std::map<std::string, std::string> out_params;

   std::visit(
   [=,&out_params]<E V>(const Params<V> &op)
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
            out_params.emplace("Data", TOMLWriteParams(op.input_xy, true));
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
            out_params.emplace("Phases", TOMLWriteValue(op.phase));
            out_params.emplace("Speeds", TOMLWriteValue(op.speed));
         }
         else if constexpr (V == SourceOption::PSD)
         {
            out_params.emplace("InputPSD", 
                                 TOMLWriteParams(op.input_psd, true));
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
                                 TOMLWriteParams(op.disc_params));
            out_params.emplace("Direction", TOMLWriteParams(op.dir_params));
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
      out_str += "\n";
   }
   if (inline_table)
   {
      out_str += "}\n";
   }

   return out_str;
}

// template<jabber_app::InputXYOption V>
// void Test(const jabber_app::InputXYParams<V> &op, 
//             const jabber_app::InputXYParams<V> &op_test)
// {
//    if constexpr (V == jabber_app::InputXYOption::Here)
//    {
//       CHECK_THAT(op.x, Catch::Matchers::Equals(op_test.x));
//       CHECK_THAT(op.y, Catch::Matchers::Equals(op_test.y));
//    }
//    else if constexpr (V == jabber_app::InputXYOption::FromCSV)
//    {
//       CHECK(op.file == op_test.file);
//    }
//    else
//    {
//       static_assert(false, "Missing params implementation.");
//    }
// }

// template<jabber_app::FunctionOption V>
// void Test(const jabber_app::FunctionParams<V> &op,
//                   const jabber_app::FunctionParams<V> &op_test)
// {
//    using namespace jabber_app;

//    if constexpr (V == FunctionOption::PiecewiseLinear || 
//                   V == FunctionOption::PiecewiseLogLog)
//    {
//       std::visit(
//       [&]<InputXYOption IV1, InputXYOption IV2>
//          (const InputXYParams<IV1> &op_ixy, 
//             const InputXYParams<IV2> &op_ixy_test) 
//       {
//          if constexpr (IV1 == IV2)
//          {
//             Test(op_ixy, op_ixy_test);
//          }
//          else
//          {
//             FAIL("Non-matching InputXYParams!");
//          }

//       }, op.input_xy, op_test.input_xy);
//    }
//    else
//    {
//       static_assert(false, "Missing params implementation.");
//    }
// }


// /// Test visitor for all parameter options.
// template<OptionEnum E, template<auto> typename Params, E... Vs>
// struct TestParamsVisitor
// {
//    const std::variant<Params<Vs>...> &opv_test;

//    constexpr E GetTestOption() const
//    {
//       return std::visit(
//          []<E V>(const Params<V> &op)
//          {
//             return V;
//          }, opv_test);
//    }

//    template<E V>
//    Params<V> GetTestParams() const
//    {
//       return std::get<Params<V>>(opv_test);
//    }

//    template<E V>
//    void operator() (const Params<V> &op)
//    {
//       REQUIRE(GetTestOption() == V);
//       Test(op, GetTestParams<V>());
//    }
// };

} // namespace jabber_test

#endif // JABBER_WITH_APP

#endif // JABBER_TEST_APP_UTILS