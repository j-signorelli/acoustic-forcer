#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <jabber.hpp>

using namespace jabber;
using namespace Catch::Matchers;
namespace jabber_test
{

TEST_CASE("Low-frequency limit", "[TransferFunctions]")
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

TEST_CASE("Flow-normal fit", "[TransferFunctions]")
{
    const double mach = GENERATE(take(3,random(1.0, 10.0)));
    const double gamma = GENERATE(take(3,random(0.5,1.6)));
    const char speed = GENERATE('S', 'F');

    SECTION("Low-frequency limit")
    {
        std::array<double,1> power_lfl{1.0};
        
        std::array<double,1> freq_lfl{0.0};
        std::array<double,1> power_fnf{1.0};

        LowFrequencyLimitTF(mach, gamma, speed, power_lfl);
        FlowNormalFitTF(mach, gamma, speed, 1.0, freq_lfl, power_fnf);

        REQUIRE(power_lfl[0] == power_fnf[0]);
    }

    SECTION("Pre-computed from t")
    {
        // Unused, but kept for documentation:
        constexpr std::array<double, 5> t_arr
        {0.46928212, 0.85471849, 0.03781777, 0.09707581, 0.57806404};
        
        constexpr std::array<double, 5> freq_nd_arr
        {0.3824347318693906, 0.6266372340276438, 0.0499083792778991,
         0.1250276007470192, 0.4292429847043615};
        
        constexpr std::array<double, 5> chi_nd_arr
        {4.270193937297611 , 2.2363223163231556, 1.0444846389474434,
         1.28539563880403  , 4.411469866465641};

        const double chi_star =
        [&]()
        {
            double c = 1.0;
            LowFrequencyLimitTF(mach, gamma, speed, {&c,1});
            return 1.0/c;
        }();

        const double delta = GENERATE(take(3,random(5e-6,1e-3)));


        std::array<double, 5> freq_arr;
        std::array<double, 5> chi_arr;
        for (std::size_t i = 0; i < 5; i++)
        {
            freq_arr[i] = freq_nd_arr[i]*delta;
            chi_arr[i] = 1.0;
        }

        
        FlowNormalFitTF(mach, gamma, speed, delta, freq_arr, chi_arr);
        
        for (std::size_t i = 0; i < 5; i++)
        {
            CHECK_THAT((1.0/chi_arr[i])/chi_star, WithinAbs(chi_nd_arr[i], 1e-12));
        }
    }

}

} // namespace jabber_test
