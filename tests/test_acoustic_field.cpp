#include "test_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <jabber.hpp>

#include <random>

using namespace jabber;

namespace test
{

/// Number of points to test with
static constexpr std::size_t NUM_PTS = 100;

/// Physical dimension to test
static constexpr int DIM = 3;
static_assert(DIM <= 3);

/// Randomizer seed
static constexpr int SEED = 17;

/// Coordinates to initialize with
static const std::array<double, NUM_PTS*DIM> coords
                           = GenerateRandomArr<NUM_PTS*DIM>(SEED,0.0,1.0);

/// Base flow to initialize with


TEST_CASE("AcousticField", "[AcousticField]")
{
   SECTION("Initialization")
   {
      AcousticField field(DIM, coords, 
   }
}

} // test
