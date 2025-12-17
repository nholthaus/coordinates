#ifndef ellipsoidTest_h__
#define ellipsoidTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <coordinate.h>
#include <units.h>

#include <initializer_list>
#include <stdexcept>
#include <type_traits>

using namespace coord;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;
using namespace units::dimensionless;

namespace
{
// The fixture for testing class Foo.
class EllipsoidTest : public ::testing::Test {
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    EllipsoidTest()
    {
        // You can do set-up work for each test here.
    }

    virtual ~EllipsoidTest()
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
    struct WrongReturnTypes
    {
        // wrong return types
        int a() const {
            return 0;
        }
        int b() const {
            return 0;
        }
        int f() const {
            return 0;
        }
        int invf() const {
            return 0;
        }
        int e() const {
            return 0;
        }
        int e2() const {
            return 0;
        }
    };

    struct MissingMembers
    {};

    struct AnEllipsoid
    {
        kilometer_t a() const {
            return 0_km;
        }
        kilometer_t b() const {
            return 0_km;
        }
        scalar_t f() const {
            return scalar_t(0.0);
        }
        scalar_t invf() const {
            return scalar_t(0.0);
        }
        scalar_t e() const {
            return scalar_t(0.0);
        }
        scalar_t e2() const {
            return scalar_t(0.0);
        }
    };
};

// TEST THAT THE ELLIPSOID CONCEPT CHECKERS DO THEIR JOB
TEST_F(EllipsoidTest, is_ellipsoid)
{
    EXPECT_TRUE(coord::traits::is_ellipsoid<ellipsoids::WGS84>::value);
    EXPECT_TRUE(coord::traits::is_ellipsoid<AnEllipsoid>::value);
    EXPECT_FALSE(coord::traits::is_ellipsoid<WrongReturnTypes>::value);
    EXPECT_FALSE(coord::traits::is_ellipsoid<MissingMembers>::value);
}

// TEST FOR THE APPROXIMATE CORRECTNESS OF ellipsoids::WGS84 DERIVED PARAMETERS
TEST_F(EllipsoidTest, WGS84)
{
    // values from Matlab: referenceEllipsoid('wgs84')

    // check source units as well as additional units
    auto a = ellipsoids::WGS84::a();
    kilometer_t a_km = ellipsoids::WGS84::a();

    EXPECT_NEAR(6378137.0, a(), 1e-1);
    EXPECT_NEAR(6378.1370, a_km(), 1e-4);

	EXPECT_NEAR(6356752.314245179, ellipsoids::WGS84::b().to<double>(), 5.0e-10);

    // these values are dimensionless
	EXPECT_NEAR(0.003352810664747, ellipsoids::WGS84::f().to<double>(), 1.0e-15);
	EXPECT_NEAR(298.257223563, ellipsoids::WGS84::invf().to<double>(), 1e-9);	// defined
	EXPECT_NEAR(0.081819190842621, ellipsoids::WGS84::e().to<double>(), 1.0e-15);
	EXPECT_NEAR(0.006694379990141, ellipsoids::WGS84::e2().to<double>(), 1.0e-15);
}

// TEST FOR THE APPROXIMATE CORRECTNESS OF ellipsoids::GRS80 DERIVED PARAMETERS
TEST_F(EllipsoidTest, GRS80)
{
    // values from Wikipedia: https://en.wikipedia.org/wiki/GRS_80

    // check source units as well as additional units
    EXPECT_NEAR(6378137.0, ellipsoids::GRS80::a().to<double>(), 1e-1);

    EXPECT_NEAR(6356752.314140347, ellipsoids::GRS80::b().to<double>(), 1e-8);

    // these values are dimensionless
	EXPECT_NEAR(0.003352810681183637418, ellipsoids::GRS80::f().to<double>(), 1e-10);
	EXPECT_NEAR(298.257222101, ellipsoids::GRS80::invf().to<double>(), 1e-10);
	EXPECT_NEAR(0.0818191910435, ellipsoids::GRS80::e().to<double>(), 1e-10);
	EXPECT_NEAR(0.0066943800229034157495749, ellipsoids::GRS80::e2().to<double>(), 1e-10);
}
}
#endif // ellipsoidTest_h__

// For Emacs
// Local Variables:
// Mode: C++
// c-basic-offset: 2
// fill-column: 116
// tab-width: 4
// End: