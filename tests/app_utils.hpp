#ifndef JABBER_APP_TEST_UTILS
#define JABBER_APP_TEST_UTILS

#ifdef JABBER_WITH_APP

#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <jabber_app.hpp>

#include <optional>

namespace jabber_test
{

/// Custom generator for all \ref jabber_app::OptionEnum enumerators.
template<jabber_app::OptionEnum T>
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

/**
 * @brief Generator-wrapper creator for list over all options by 
 *  \ref jabber_app::OptionEnum. Use by \c GENERATE(options<E>()).
 *
 */
template <jabber_app::OptionEnum E>
Catch::Generators::GeneratorWrapper<E> options()
{
    return Catch::Generators::GeneratorWrapper<E>(
               Catch::Detail::make_unique<OptionGenerator<E>>());
}

/// Random generator for a \ref jabber_app::OptionEnum enumerator.
template<jabber_app::OptionEnum E>
class RandomOptionGenerator : public Catch::Generators::IGenerator<E>
{
private:
   Catch::Generators::GeneratorWrapper<std::uint8_t> ri_gen_;
   E option_;
public:
   RandomOptionGenerator()
   : ri_gen_(Catch::Generators::random<std::uint8_t>(0, 
               static_cast<std::uint8_t>(E::Size)-1))
   { 
      static_cast<void>(next());
   }

   E const& get() const override
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

/**
 * @brief Generator-wrapper creator for random option of 
 * \ref jabber_app::OptionEnum. Use by \c random_option<E>().
 */
template<jabber_app::OptionEnum E>
Catch::Generators::GeneratorWrapper<E> random_option() 
{
   return Catch::Generators::GeneratorWrapper<E>(
            Catch::Detail::make_unique<RandomOptionGenerator<E>>());
}

template<OptionEnum E>
class

/**
 * @brief Generator for a random option parameters variant for a given
 * \ref jabber_app::OptionEnum.
 * 
 * @details If option is provided at construction, params type will stay 
 * same throughout object life. Otherwise, params type is randomized 
 * with every call to \ref next().
 */
template<jabber_app::OptionEnum E>
class RandomParamsGenerator
   : public Catch::Generators::IGenerator<jabber_app::OptionParamsVar<E>>
{
private:
   const std::optional<E> fixed_option_;
   RandomOptionGenerator<E> ropt_gen_;

   <E> rand_;
   jabber_app::OptionParamsVar<E> opv_;

public:
   RandomParamsVarGenerator()
   : fixed_option_(std::nullopt)
   {
      static_cast<void*>(next());
   }

   RandomParamsVarGenerator(const E &option)
   : fixed_option_(option)
   {
      static_cast<void*>(next());
   }

   jabber_app::OptionParamsVar<E> const& get() const override
   {
      return opv_;
   }

   bool next() override
   {
      opv_ = rand_.next(fixed_option_ ? 
                        *fixed_option_ 
                        : ropt_gen_.get());
   }

}


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
 * @todo REFACTOR!
 * @brief Random parameter generator for option-ed parameters.
 * 
 * @details This is a simple way to get random parameters for an
 * option-ed set. The return type is hardcoded to be the variant
 * associated with the given jabber_app::OptionEnum \p T.
 * 
 * @tparam T      Option enum (Such as InputXYOption).
 * @param option  [Optional] Specific option to generate random
 *                parameters for.
 * @return auto   Generator wrapper of parameters variant.
 */
template<jabber_app::OptionEnum T>
auto random_params(std::optioonalT option=random_option<T>().get()) 
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
