#ifndef JABBER_TEST_APP_UTILS
#define JABBER_TEST_APP_UTILS

#ifdef JABBER_WITH_APP
#include "utils.hpp"
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
 * @brief Generator-wrapper creator for random params of 
 * \ref OptionEnum. Use by \c random_params<E>(E::V) or with varying param
 * type by \c random_params<E>().
 */
template<OptionEnum E>
constexpr auto random_params(std::optional<E> V=std::nullopt)
{
   using ReturnType = 
   decltype([]()
   {
      if constexpr (std::same_as<E, jabber_app::InputXYOption>)
      {
         return jabber_app::InputXYParamsVariant{};
      }

   }());

   using RandomGeneratorType = 
   decltype([]()
   {
      if constexpr (std::same_as<E, jabber_app::InputXYOption>)
      {
         return RandomInputXYGenerator();
      }

   }());

   return Catch::Generators::GeneratorWrapper<ReturnType>(
            Catch::Detail::make_unique<RandomGeneratorType>(V));
}

} // namespace jabber_test

#endif // JABBER_WITH_APP

#endif // JABBER_TEST_APP_UTILS