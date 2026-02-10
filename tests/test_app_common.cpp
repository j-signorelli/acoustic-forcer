#ifdef JABBER_WITH_APP

#include "utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <jabber_app.hpp>

using namespace jabber_app;
using namespace Catch::Matchers;

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

} // jabber_test

#endif // JABBER_WITH_APP
