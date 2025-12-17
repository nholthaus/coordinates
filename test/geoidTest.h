#ifndef geoidTest_h__
#define geoidTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace coord;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

namespace
{
	// The fixture for testing
	class GeoidTest : public ::testing::Test {
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
		EXPECT_TRUE(coord::traits::is_geoid<geoids::EGM96>::value);
		EXPECT_TRUE(coord::traits::is_geoid<geoids::NULL_GEOID>::value);
		EXPECT_TRUE(coord::traits::is_geoid<geoids::GEOID12A>::value);
		EXPECT_FALSE(coord::traits::is_geoid<ellipsoids::WGS84>::value);
	}

	TEST_F(GeoidTest, EGM96)
	{
		// Test the dimensions, and (near the) 4 corners of the lookup table to make sure we haven't screwed it up.
		// We don't test the actual corners because the NGA's online calculator can't do it :P
		// God help us if someone accidentally edits the LUT in the middle.

		// truth values are from http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/intpt.html and
		// http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/outintpt.dat
		using degree_t = units::angle::degree_t;

		meter_t test1 = geoids::EGM96::undulation(38.628155_deg, 269.779155_deg);
		meter_t test2 = geoids::EGM96::undulation(-14.621217_deg, 305.021114_deg);
		meter_t test3 = geoids::EGM96::undulation(46.874319_deg, 102.448729_deg);
		meter_t test4 = geoids::EGM96::undulation(-23.617446_deg, 133.874712_deg);
		meter_t test5 = geoids::EGM96::undulation(38.625473_deg, 359.999500_deg);
		meter_t test6 = geoids::EGM96::undulation(-00.466744_deg, 0.002300_deg);
		meter_t boston = geoids::EGM96::undulation(42.3550_deg, -71.0656_deg);
		meter_t nevada = geoids::EGM96::undulation(37.2350_deg, -115.8111_deg);
		meter_t exact = geoids::EGM96::undulation(25.75_deg, 121.25_deg);

		// test accuracy to ~1cm
		EXPECT_NEAR(-31.628, test1(), 1.1e-2);
		EXPECT_NEAR(-2.969, test2(), 1.1e-2);
		EXPECT_NEAR(-43.575, test3(), 1.1e-2);
		EXPECT_NEAR(15.871, test4(), 1.1e-2);
		EXPECT_NEAR(50.066, test5(), 1.1e-2);
		EXPECT_NEAR(17.329, test6(), 1.1e-2);
		EXPECT_NEAR(-28.550, boston(), 1.1e-2);
		EXPECT_NEAR(-26.449, nevada(), 1.1e-2);
		EXPECT_NEAR(17.502, exact(), 1.1e-2);
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

		EXPECT_NEAR(-25.527, (geoids::GEOID12A::undulation(25.7617_deg, -80.1918_deg).to<double>()),  5.0e-4);	// Miami
		EXPECT_NEAR(-32.052, (geoids::GEOID12A::undulation(38.9072_deg, -77.0369_deg).to<double>()),  5.0e-4);	// D.C
		EXPECT_NEAR(-27.734, (geoids::GEOID12A::undulation(42.3601_deg, -71.0589_deg).to<double>()),  5.0e-4);	// Boston
		EXPECT_NEAR(-33.583, (geoids::GEOID12A::undulation(41.8781_deg, -87.6298_deg).to<double>()),  5.0e-4);	// Chicago
		EXPECT_NEAR(-24.073, (geoids::GEOID12A::undulation(31.7619_deg, -106.4850_deg).to<double>()), 7.0e-4);	// El Paso
		EXPECT_NEAR(-16.815, (geoids::GEOID12A::undulation(43.6187_deg, -116.2146_deg).to<double>()), 5.0e-4);	// Boise
		EXPECT_NEAR(-35.105, (geoids::GEOID12A::undulation(34.0522_deg, -118.2437_deg).to<double>()), 5.0e-4);	// Los Angeles
		EXPECT_NEAR(-32.518, (geoids::GEOID12A::undulation(37.7749_deg, -122.4194_deg).to<double>()), 5.0e-4);	// San Francisco
		EXPECT_NEAR(-22.832, (geoids::GEOID12A::undulation(45.5231_deg, -122.6765_deg).to<double>()), 7.0e-4);	// Portland
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

		EXPECT_NEAR(-27.169, (geoids::USGG2012::undulation(25.7617_deg, -80.1918_deg).to<double>()), 5.0e-4);		// Miami
		EXPECT_NEAR(-32.932, (geoids::USGG2012::undulation(38.9072_deg, -77.0369_deg).to<double>()), 5.0e-4);		// D.C
		EXPECT_NEAR(-28.595, (geoids::USGG2012::undulation(42.3601_deg, -71.0589_deg).to<double>()), 5.0e-4);		// Boston
		EXPECT_NEAR(-33.912, (geoids::USGG2012::undulation(41.8781_deg, -87.6298_deg).to<double>()), 5.0e-4);		// Chicago
		EXPECT_NEAR(-24.805, (geoids::USGG2012::undulation(31.7619_deg, -106.4850_deg).to<double>()), 5.0e-4);	// El Paso
		EXPECT_NEAR(-16.434, (geoids::USGG2012::undulation(43.6187_deg, -116.2146_deg).to<double>()), 5.0e-4);	// Boise
		EXPECT_NEAR(-35.202, (geoids::USGG2012::undulation(34.0522_deg, -118.2437_deg).to<double>()), 5.0e-4);	// Los Angeles
		EXPECT_NEAR(-32.208, (geoids::USGG2012::undulation(37.7749_deg, -122.4194_deg).to<double>()), 5.0e-4);	// San Francisco
		EXPECT_NEAR(-22.073, (geoids::USGG2012::undulation(45.5231_deg, -122.6765_deg).to<double>()), 5.0e-4);	// Portland
	}
}


#endif // geoidTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: