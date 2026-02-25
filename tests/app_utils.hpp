#ifndef JABBER_APP_TEST_UTILS
#define JABBER_APP_TEST_UTILS

#ifdef JABBER_WITH_APP

#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <jabber_app.hpp>

namespace jabber_test
{

// Helper for getting a vector as a string for TOML config files.
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


/// Get the option, given the variant.
template<OptionEnum E, E... Vs, template<E...> typename Params>
constexpr E GetOption(const std::variant<Params<Vs>...> &var)
{
   // Get the type
   return std::visit(
            [&]<E V>(const Params<V> &val)
            {
               return V;
            }, var);
}

/// Get the name, given the option.
template<OptionEnum E>
constexpr std::string_view GetName(const std::array<std::string_view, 
                                    static_cast<std::size_t>(E::Size)> &names,
                                   const E &option)
{
   return names[static_cast<std::size_t>(option)];
}

/**
 * @brief Get the name, given the params.
 * 
 * @details This is lazy-use function. If you have the specific params, 
 * you have \p V and can use the former \ref GetName().
 */
template<OptionEnum E, E V, template<E> typename Params>
constexpr std::string_view GetName(const std::array<std::string_view, 
                                    static_cast<std::size_t>(E::Size)> &names,
                                   const Params<V> &params)
{
   return GetName(names, V);
}

/**
 * @brief Get the option, given the params.
 * 
 * @details This is lazy-use function. If you have the params, you have \p V.
 */
template<OptionEnum E, E V, template<E> typename Params>
constexpr E GetOption(const Params<V> &params)
{
   return V;
}


/// InputXYParams TOML writer visitor.
struct TOMLInputXYWriterVisitor
{
   template<jabber_app::InputXYOption I>
   std::string operator() (const jabber_app::InputXYParams<I> &ip)
   {
      constexpr std::string_view kName = GetName(jabber_app::InputXYNames, I);
      std::string out_str = std::format("Data={Type=\"{}\",\n", kName);

      if constexpr (I == jabber_app::InputXYOption::Here)
      {
         out_str += std::format( "X={},\n"
                                 "Y={}\n",
                                 TOMLWriteVector(ip.x), 
                                 TOMLWriteVector(ip.y));
      }
      else if constexpr (I == jabber_app::InputXYOption::FromCSV)
      {
         out_str += std::format("File=\"{}\"\n",ip.file);
      }
      else
      {
         static_assert(false, "Missing params implementation.");
      }
      out_str += "}"
      return out_str;
   }
};

/// InputXYParams test visitor.
struct TestInputXYVisitor
{
   const jabber_app::InputXYParamsVariant &ipv;

   template<jabber_app::InputXYOption I>
   void operator() (const jabber_app::InputXYParams<I> &ip_test) const
   {
      REQUIRE(GetOption(ipv) == I);
      const jabber_app::InputXYParams<I> &ip = 
         std::get<jabber_app::InputXYParams<I>>(ipv);

      if constexpr (I == jabber_app::InputXYOption::Here)
      {
         CHECK_THAT(ip.x, Catch::Matchers::Equals(ip_test.x));
         CHECK_THAT(ip.y, Catch::Matchers::Equals(ip_test.y));
      }
      else if constexpr (I == jabber_app::InputXYOption::FromCSV)
      {
         CHECK(ip.file == ip_test.file);
      }
      else
      {
         static_assert(false, "Missing params implementation.");
      }
   }
};

/// FunctionParams TOML writer visitor.
struct TOMLFunctionWriterVisitor
{
   template<jabber_app::FunctionOption F>
   std::string operator() (const jabber_app::FunctionParams<F> &fp)
   {
      constexpr std::string_view kName = GetName(jabber_app::FunctionNames, F);
      std::string out_str = std::format("Type=\"{}\",\n", kName);

      if constexpr (F == jabber_app::FunctionOption::PiecewiseLinear ||
                     F == jabber_app::FunctionOption::PiecewiseLogLog)
      {
         out_str += std::format( "Data={{{}}},\n",
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

/// FunctionParams test visitor.
struct TestFunctionVisitor
{
   const jabber_app::FunctionParamsVariant &fpv;

   template<jabber_app::FunctionOption F>
   void operator() (const jabber_app::FunctionParams<F> &fp_test) const
   {
      REQUIRE(GetOption(fpv) == F);
      const jabber_app::FunctionParams<F> &fp = 
         std::get<jabber_app::FunctionParams<F>>(fpv);

      if constexpr (F == jabber_app::FunctionOption::PiecewiseLinear || 
                     F == jabber_app::FunctionOption::PiecewiseLogLog)
      {
         std::visit(TestInputXYVisitor{fp.input_xy}, fp_test.input_xy);
      }
   }
};

/// Custom generator for all \ref OptionEnum enumerators.
template<OptionEnum T>
class OptionGenerator : public Catch::Generators::IGenerator<T>
{
private:
   T enumerator_val_;
   std::uint8_t enum_idx_ = 0;
public:
   bool next() override
   {
      if (++enum_idx_ < static_cast<std::uint8_t>(T::Size))
      {
         enumerator_val_ = static_cast<T>(enum_idx_);
         return true;
      }
      else
      {
         return false;
      }
   }

   T const& get() const override
   {
      return enumerator_val_;
   }
};

/// Generator-wrapper creator for list over all options by \ref OptionEnum.
template <OptionEnum T>
Catch::Generators::GeneratorWrapper<T> options()
{
    return Catch::Generators::GeneratorWrapper<T>(
               Catch::Detail::make_unique<OptionGenerator<T>>());
}

/// Random generator for a \ref OptionEnum enumerator.
template<OptionEnum T>
class RandomOptionGenerator : public Catch::Generators::IGenerator<T>
{
private:
   Catch::Generators::GeneratorWrapper<std::uint8_t> ri_gen_;
   T option_;
public:
   RandomOptionGenerator()
   : ri_gen_(Catch::Generators::random<std::uint8_t>(0, 
               static_cast<std::uint8_t>(T::Size)-1))
   { 
      static_cast<void>(next());
   }

   T const& get() const override
   {
      return option_;
   }

   bool next() override
   {
      ri_gen_.next();
      option_ = static_cast<T>(ri_gen_.get());
      return true;
   }
};

/// Generator-wrapper creator for random option of \ref OptionEnum.
template<OptionEnum T>
Catch::Generators::GeneratorWrapper<T> random_option() 
{
   return Catch::Generators::GeneratorWrapper<T>(
            Catch::Detail::make_unique<RandomOptionGenerator<T>>());
}

/// Random generator for jabber_app::InputXYParamsVariant.
class RandomInputXYGenerator 
   : public Catch::Generators::IGenerator<jabber_app::InputXYParamsVariant>
{
private:

   jabber_app::InputXYOption ixy_option_;

   jabber_app::InputXYParamsVariant ipv_;

   // For InputXYOption::Here
   static constexpr std::size_t kSize = 20;
   Catch::Generators::ChunkGenerator<double> x_gen_, y_gen_;

   // For InputXYOption::FromCSV
   Catch::Generators::GeneratorWrapper<int> ri_file_suffix_gen_;

public:
   RandomInputXYGenerator()
   : RandomInputXYGenerator(static_cast<jabber_app::InputXYOption>(0))
   { }

   RandomInputXYGenerator(jabber_app::InputXYOption ixy_option);

   void SetOption(jabber_app::InputXYOption ixy_option)
   {
      ixy_option_ = ixy_option;
   }

   jabber_app::InputXYParamsVariant const& get() const override
   {
      return ipv_;
   }

   bool next() override;
};

/// Random generator for jabber_app::FunctionParamsVariant.
class RandomFunctionGenerator 
   : public Catch::Generators::IGenerator<jabber_app::FunctionParamsVariant>
{
private:

   jabber_app::FunctionOption f_option_;

   jabber_app::FunctionParamsVariant fpv_;

   // For FunctionOption::PiecewiseLinear and FunctionOption::PiecewiseLogLog
   RandomOptionGenerator<jabber_app::InputXYOption> rixy_opt_gen_;
   RandomInputXYGenerator rixy_gen_;

public:
   RandomFunctionGenerator()
   : RandomFunctionGenerator(static_cast<jabber_app::FunctionOption>(0))
   { }

   RandomFunctionGenerator(jabber_app::FunctionOption f_option);

   void SetOption(jabber_app::FunctionOption f_option)
   {
      f_option_ = f_option;
   }

   jabber_app::FunctionParamsVariant const& get() const override
   {
      return fpv_;
   }

   bool next() override;
};

/**
 * @brief Random parameter generator for option-ed parameters.
 * 
 * @details This is a simple way to get random parameters for an
 * option-ed set. The return type is hardcoded to be the variant
 * associated with the given OptionEnum \p T.
 * 
 * @tparam T      Option enum (Such as InputXYOption).
 * @param option  [Optional] Specific option to generate random
 *                parameters for.
 * @return auto   Generator wrapper of parameters variant.
 */
template<OptionEnum T>
auto random_params(T option=random_option<T>().get()) 
{
   using namespace Catch::Generators;
   using namespace Catch::Detail;
   using namespace jabber_app;

   if constexpr(std::same_as<T, InputXYOption>)
   {
      return GeneratorWrapper<InputXYParamsVariant>
               (make_unique<RandomInputXYGenerator>(option));
   }
   else if constexpr(std::same_as<T, FunctionOption>)
   {
      return GeneratorWrapper<FunctionParamsVariant>
               (make_unique<RandomFunctionGenerator>(option));
   }
}

} // namespace jabber_test

#endif // JABBER_WITH_APP

#endif // JABBER_APP_TEST_UTILS
