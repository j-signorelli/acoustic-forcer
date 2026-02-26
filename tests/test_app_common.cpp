#ifdef JABBER_WITH_APP

#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <jabber_app.hpp>

#include <filesystem>
#include <fstream>

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
   const InputXY option = GENERATE(options<InputXY>());
   const std::uint8_t idx = static_cast<std::uint8_t>(option);

   constexpr std::size_t kN = 20;

   DYNAMIC_SECTION(InputXYNames[idx])
   {
      InputXYParamsVariant ipv = GenerateRandomInputXY(option, kSeed);
   
      std::vector<double> x = GenerateRandomVec<kN>(kSeed, 10, 100e3);
      std::vector<double> y = GenerateRandomVec<kN>(kSeed, 1e-9, 1e-12);

      // Pre-process
      std::visit(
      overloads
      {
      [&](InputXYParams<InputXY::Here> &ip)
      {
         ip.x = x;
         ip.y = y;
      },
      [&](InputXYParams<InputXY::FromCSV> &ip)
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
      [&]<InputXY I>(const InputXYParams<I> &ip)
      {
         if constexpr (I == InputXY::FromCSV)
         {
            std::filesystem::remove(ip.file);
         }
      }, ipv);
   }
}

} // jabber_test

#endif // JABBER_WITH_APP
