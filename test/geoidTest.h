#ifndef geoidTest_h__
#define geoidTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <coord/geoid.h>
#include <stdexcept>
#include <type_traits>

using namespace coord;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

// The fixture for testing
class GeoidTest : public ::testing::Test
{
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    GeoidTest()
    {
        // You can do set-up work for each test here.
    }

    virtual ~GeoidTest()
    {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    virtual void SetUp()
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    virtual void TearDown()
    {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case for Foo.
};

// TEST THE IS_GEOID CONCEPT CHECKER
TEST_F(GeoidTest, is_geoid)
{
    EXPECT_TRUE(coord::traits::is_geoid<geoids::EGM96>);
    EXPECT_TRUE(coord::traits::is_geoid<geoids::NULL_GEOID>);
    EXPECT_TRUE(coord::traits::is_geoid<geoids::GEOID12A>);
    EXPECT_FALSE(coord::traits::is_geoid<ellipsoids::WGS84>);
}

TEST_F(GeoidTest, EGM96)
{
    // Test the dimensions, and (near the) 4 corners of the lookup table to make sure we haven't screwed it up.
    // We don't test the actual corners because the NGA's online calculator can't do it :P
    // God help us if someone accidentally edits the LUT in the middle.

    // truth values are from http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/intpt.html and
    // http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/outintpt.dat

    constexpr auto test1 = geoids::EGM96::undulation(38.628155_deg, 269.779155_deg);
    constexpr auto test2 = geoids::EGM96::undulation(-14.621217_deg, 305.021114_deg);
    constexpr auto test3 = geoids::EGM96::undulation(46.874319_deg, 102.448729_deg);
    constexpr auto test4 = geoids::EGM96::undulation(-23.617446_deg, 133.874712_deg);
    constexpr auto test5 = geoids::EGM96::undulation(38.625473_deg, 359.999500_deg);
    constexpr auto test6 = geoids::EGM96::undulation(-00.466744_deg, 0.002300_deg);
    constexpr auto boston = geoids::EGM96::undulation(42.3550_deg, -71.0656_deg);
    constexpr auto nevada = geoids::EGM96::undulation(37.2350_deg, -115.8111_deg);
    constexpr auto exact = geoids::EGM96::undulation(25.75_deg, 121.25_deg);

    // test accuracy to ~1cm
    EXPECT_NEAR_UNITS(-31.628_m, test1, 1.1_cm);
    EXPECT_NEAR_UNITS(-2.969_m, test2, 1.1_cm);
    EXPECT_NEAR_UNITS(-43.575_m, test3, 1.1_cm);
    EXPECT_NEAR_UNITS(15.871_m, test4, 1.1_cm);
    EXPECT_NEAR_UNITS(50.066_m, test5, 1.1_cm);
    EXPECT_NEAR_UNITS(17.329_m, test6, 1.1_cm);
    EXPECT_NEAR_UNITS(-28.550_m, boston, 1.1_cm);
    EXPECT_NEAR_UNITS(-26.449_m, nevada, 1.1_cm);
    EXPECT_NEAR_UNITS(17.502_m, exact, 1.1_cm);
}

TEST_F(GeoidTest, GEOID12A)
{
    // Truth values are from: http://www.ngs.noaa.gov/cgi-bin/GEOID_STUFF/geoid12A_prompt1.prl

    // out of bounds of the geoid LUT
    EXPECT_THROW(geoids::GEOID12A::undulation(23_deg, -130_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(23_deg, -60_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(24_deg, -131_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(24_deg, -59_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(59_deg, -130_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(59_deg, -60_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(58_deg, -131_deg), std::runtime_error);
    EXPECT_THROW(geoids::GEOID12A::undulation(58_deg, -59_deg), std::runtime_error);

    constexpr auto Miami = geoids::GEOID12A::undulation(25.7617_deg, -80.1918_deg);
    constexpr auto DC = geoids::GEOID12A::undulation(38.9072_deg, -77.0369_deg);
    constexpr auto Boston = geoids::GEOID12A::undulation(42.3601_deg, -71.0589_deg);
    constexpr auto Chicago = geoids::GEOID12A::undulation(41.8781_deg, -87.6298_deg);
    constexpr auto ElPaso = geoids::GEOID12A::undulation(31.7619_deg, -106.4850_deg);
    constexpr auto Boise = geoids::GEOID12A::undulation(43.6187_deg, -116.2146_deg);
    constexpr auto LosAngeles = geoids::GEOID12A::undulation(34.0522_deg, -118.2437_deg);
    constexpr auto SanFrancisco = geoids::GEOID12A::undulation(37.7749_deg, -122.4194_deg);
    constexpr auto Portland = geoids::GEOID12A::undulation(45.5231_deg, -122.6765_deg);

    EXPECT_NEAR_UNITS(-25.527_m, Miami, 1_mm); // Miami
    EXPECT_NEAR_UNITS(-32.052_m, DC, 1_mm); // D.C
    EXPECT_NEAR_UNITS(-27.734_m, Boston, 1_mm); // Boston
    EXPECT_NEAR_UNITS(-33.583_m, Chicago, 1_mm); // Chicago
    EXPECT_NEAR_UNITS(-24.073_m, ElPaso, 1_mm); // El Paso
    EXPECT_NEAR_UNITS(-16.815_m, Boise, 1_mm); // Boise
    EXPECT_NEAR_UNITS(-35.105_m, LosAngeles, 1_mm);// Los Angeles
    EXPECT_NEAR_UNITS(-32.518_m, SanFrancisco, 1_mm);// San Francisco
    EXPECT_NEAR_UNITS(-22.832_m, Portland, 1_mm); // Portland
}

TEST_F(GeoidTest, USGG2012)
{
    // Truth values are from: http://www.ngs.noaa.gov/cgi-bin/GEOID_STUFF/usgg2012_prompt1.prl

    // out of bounds of the geoid LUT
    EXPECT_THROW(geoids::USGG2012::undulation(23_deg, -130_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(23_deg, -60_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(24_deg, -131_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(24_deg, -59_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(59_deg, -130_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(59_deg, -60_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(58_deg, -131_deg), std::runtime_error);
    EXPECT_THROW(geoids::USGG2012::undulation(58_deg, -59_deg), std::runtime_error);

    constexpr auto Miami = geoids::USGG2012::undulation(25.7617_deg, -80.1918_deg);
    constexpr auto DC = geoids::USGG2012::undulation(38.9072_deg, -77.0369_deg);
    constexpr auto Boston = geoids::USGG2012::undulation(42.3601_deg, -71.0589_deg);
    constexpr auto Chicago = geoids::USGG2012::undulation(41.8781_deg, -87.6298_deg);
    constexpr auto ElPaso = geoids::USGG2012::undulation(31.7619_deg, -106.4850_deg);
    constexpr auto Boise = geoids::USGG2012::undulation(43.6187_deg, -116.2146_deg);
    constexpr auto LosAngeles = geoids::USGG2012::undulation(34.0522_deg, -118.2437_deg);
    constexpr auto SanFrancisco = geoids::USGG2012::undulation(37.7749_deg, -122.4194_deg);
    constexpr auto Portland = geoids::USGG2012::undulation(45.5231_deg, -122.6765_deg);

    EXPECT_NEAR_UNITS(-27.169_m, Miami, 1_mm); // Miami
    EXPECT_NEAR_UNITS(-32.932_m, DC, 1_mm); // D.C
    EXPECT_NEAR_UNITS(-28.595_m, Boston, 1_mm); // Boston
    EXPECT_NEAR_UNITS(-33.912_m, Chicago, 1_mm); // Chicago
    EXPECT_NEAR_UNITS(-24.805_m, ElPaso, 1_mm); // El Paso
    EXPECT_NEAR_UNITS(-16.434_m, Boise, 1_mm); // Boise
    EXPECT_NEAR_UNITS(-35.202_m, LosAngeles, 1_mm);// Los Angeles
    EXPECT_NEAR_UNITS(-32.208_m, SanFrancisco, 1_mm);// San Francisco
    EXPECT_NEAR_UNITS(-22.073_m, Portland, 1_mm); // Portland
}

#endif // geoidTest_h__
