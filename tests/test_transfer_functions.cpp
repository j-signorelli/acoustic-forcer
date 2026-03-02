#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <jabber.hpp>

using namespace jabber;
using namespace Catch::Matchers;
namespace jabber_test
{

TEST_CASE("Low-frequency limit", "[TransferFunction]")
{
    std::array<double,1> power{1.0};

    SECTION("Slow")
    {
        LowFrequencyLimitTF(6.0, 1.4, 'S', power);
        const double chi_star = 1.0/power[0];
        CHECK_THAT(chi_star, WithinAbs(0.23175337604870483, 1e-14));
    }
    SECTION("Fast")
    {
        LowFrequencyLimitTF(6.0, 1.4, 'F', power);
        const double chi_star = 1.0/power[0];
        CHECK_THAT(chi_star, WithinAbs(0.907933119227385, 1e-14));
    }
}

} // namespace jabber_test
