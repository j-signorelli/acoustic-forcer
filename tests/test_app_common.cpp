#ifdef JABBER_WITH_APP

#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <jabber_app.hpp>

#include <filesystem>
#include <fstream>

using namespace jabber;
using namespace jabber_app;
using namespace Catch::Matchers;

// Helper type for the std::visit
// (https://en.cppreference.com/w/cpp/utility/variant/visit)
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };


namespace jabber_test
{

static constexpr int SEED = 0;
static constexpr int SIZE = 3;

TEST_CASE("Normalize", "[App]")
{   
   std::array<double, SIZE> vec = GenerateRandomArr<SIZE>(SEED, 0.0, 1.0);
   std::array<double, SIZE> norm_vec;

   Normalize(vec, norm_vec);

   // Check vec dot norm_vec == ||vec||
   const double vec_mag = std::sqrt(std::inner_product(vec.begin(), 
                                                      vec.end(), 
                                                      vec.begin(), 
                                                      0.0));
   const double dot_product = std::inner_product(vec.begin(), 
                                                   vec.end(), 
                                                   norm_vec.begin(),
                                                   0.0);
   CAPTURE(vec, norm_vec);
   REQUIRE_THAT(dot_product, WithinULP(vec_mag, 0));
}

TEST_CASE("InputXYVisitor", "[App]")
{
   constexpr int kSeed = 0;
   const InputXYOption kOption = GENERATE(options<InputXYOption>());
   const std::uint8_t kIdx = static_cast<std::uint8_t>(kOption);

   constexpr std::size_t kN = 20;

   InputXYParamsVariant ipv = GenerateRandomInputXY(kOption, kSeed);

   std::vector<double> x = GenerateRandomVec<kN>(kSeed, 10, 100e3);
   std::vector<double> y = GenerateRandomVec<kN>(kSeed, 1e-9, 1e-12);

   DYNAMIC_SECTION(InputXYNames[kIdx])
   {

      // Pre-process
      std::visit(
      overloads
      {
      [&](InputXYParams<InputXYOption::Here> &ip)
      {
         ip.x = x;
         ip.y = y;
      },
      [&](InputXYParams<InputXYOption::FromCSV> &ip)
      {
         // Write sample CSV file.
         std::string file_name = std::filesystem::temp_directory_path() 
                                 / "jabber_unit_test_XXXXXX";
         mkstemp(file_name.data());

         std::ofstream file(file_name);
         for (std::size_t i = 0; i < kN; i++)
         {
            file << std::format("{},{}", x[i], y[i]) << std::endl;
         }

         ip.file = file_name;
      }
      }, ipv);

      // Apply the visit
      std::vector<double> x_test, y_test;
      std::visit(InputXYVisitor{x_test, y_test}, ipv);

      CHECK_THAT(x, Equals(x_test));
      CHECK_THAT(y, Equals(y_test));

      // Delete any written files...
      std::visit(
      [&]<InputXYOption I>(const InputXYParams<I> &ip)
      {
         if constexpr (I == InputXYOption::FromCSV)
         {
            std::filesystem::remove(ip.file);
         }
      }, ipv);
   }
}

// TEMPLATE_TEST_CASE("FunctionVisitor", "[App]", Function, BasePSD)
// {
//    constexpr int kSeed = 0;
//    const FunctionOption option = GENERATE(options<FunctionOption>());
//    const std::uint8_t idx = static_cast<std::uint8_t>(option);
//    DYNAMIC_SECTION(FunctionNames[idx])
//    {
//       FunctionParamsVariant fpv = GenerateRandomFunction(option, kSeed);
      
//       // Initialize the ptr to test against.
//       std::unique_ptr<TestType> T_ptr;
//       std::visit(
//       [&]<FunctionOption F>(const FunctionParams<F> &fp)
//       {
//          // Potential x,y vars to use depending on input
//          std::vector<double> x, y;
//          if constexpr (F == FunctionOption::PiecewiseLinear || 
//                           F == FunctionOption::PiecewiseLogLog)
//          {
//             std::visit(InputXYVisitor{x,y}, fp.input_xy);
//          }

//          if constexpr (std::is_same_v<TestType, Function>)
//          {
//             if constexpr (F == FunctionOption::PiecewiseLinear)
//             {
//                T_ptr = std::make_unique<PWLinear>(x,y);
//             }
//             else if constexpr (F == FunctionOption::PiecewiseLogLog)
//             {
//                T_ptr = std::make_unique<PWLogLog>(x,y);
//             }
//          }
//          else // Else must be BasePSD
//          {
//             if constexpr (F == FunctionOption::PiecewiseLinear)
//             {
//                T_ptr = std::make_unique<PWLinearPSD>(x,y);
//             }
//             else if constexpr (F == FunctionOption::PiecewiseLogLog)
//             {
//                T_ptr = std::make_unique<PWLogLogPSD>(x,y);
//             }
//          }

//       }, fpv);


//       // Apply the visit
//       std::unique_ptr<TestType> T_ptr_test;
//       std::visit(FunctionVisitor{T_ptr_test}, fpv);

//       // Check if they are the same type



//    }
// }

} // jabber_test

#endif // JABBER_WITH_APP
