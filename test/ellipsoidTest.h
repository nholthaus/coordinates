#ifndef ellipsoidTest_h__
#define ellipsoidTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <coord/ellipsoid.h>
#include <units.h>

#include "gtest_units.h"

using namespace coord;
using namespace units;
using namespace units::length;
using namespace units::angle;
using namespace units::time;

// The fixture for testing class Foo.
class EllipsoidTest : public ::testing::Test
{
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
        int a() const
        {
            return 0;
        }

        int b() const
        {
            return 0;
        }

        int f() const
        {
            return 0;
        }

        int invf() const
        {
            return 0;
        }

        int e() const
        {
            return 0;
        }

        int e2() const
        {
            return 0;
        }
    };

    struct MissingMembers
    {
    };

    struct AnEllipsoid
    {
        static constexpr kilometers<> a()
        {
            return 0_km;
        }

        static constexpr kilometers<> b()
        {
            return 0_km;
        }

        static constexpr dimensionless<> f()
        {
            return 0.0;
        }

        static constexpr dimensionless<> invf()
        {
            return 0.0;
        }

        static constexpr dimensionless<> e()
        {
            return 0.0;
        }

        static constexpr dimensionless<> e2()
        {
            return 0.0;
        }
    };
};

template <auto> struct RequireConstexpr {};

// TEST THAT THE ELLIPSOID CONCEPT CHECKERS DO THEIR JOB
TEST_F(EllipsoidTest, is_ellipsoid)
{
    EXPECT_TRUE(coord::traits::is_ellipsoid<WGS84>);
    EXPECT_TRUE(coord::traits::is_ellipsoid<AnEllipsoid>);
    EXPECT_FALSE(coord::traits::is_ellipsoid<WrongReturnTypes>);
    EXPECT_FALSE(coord::traits::is_ellipsoid<MissingMembers>);
}

TEST_F(EllipsoidTest, constexpr)
{
    static_assert(coord::traits::is_ellipsoid<WGS84>);

    using _a = RequireConstexpr<WGS84::a()>;
    using _b = RequireConstexpr<WGS84::b()>;
    using _f = RequireConstexpr<WGS84::f()>;
    using _invf = RequireConstexpr<WGS84::invf()>;
    using _e = RequireConstexpr<WGS84::e()>;
    using _e2 = RequireConstexpr<WGS84::e2()>;
}
// TEST FOR THE APPROXIMATE CORRECTNESS OF WGS84 DERIVED PARAMETERS
TEST_F(EllipsoidTest, WGS84)
{
    // values from Matlab: referenceEllipsoid('wgs84')

    // check source units as well as additional units
    auto a = WGS84::a();
    kilometers a_km = WGS84::a();

    EXPECT_NEAR_UNITS(6378137.0_m, a, 1e-1_m);
    EXPECT_NEAR_UNITS(6378.1370_km, a_km, 1e-4_km);

    EXPECT_NEAR(6356752.314245179, WGS84::b().to<double>(), 5.0e-10);

    // these values are dimensionless
    EXPECT_NEAR(0.003352810664747, WGS84::f().to<double>(), 1.0e-15);
    EXPECT_NEAR(298.257223563, WGS84::invf().to<double>(), 1e-9); // defined
    EXPECT_NEAR(0.081819190842621, WGS84::e().to<double>(), 1.0e-15);
    EXPECT_NEAR(0.006694379990141, WGS84::e2().to<double>(), 1.0e-15);
}

// TEST FOR THE APPROXIMATE CORRECTNESS OF GRS80 DERIVED PARAMETERS
TEST_F(EllipsoidTest, GRS80)
{
    // values from Wikipedia: https://en.wikipedia.org/wiki/GRS_80

    // check source units as well as additional units
    EXPECT_NEAR(6378137.0, GRS80::a().to<double>(), 1e-1);

    EXPECT_NEAR(6356752.314140347, GRS80::b().to<double>(), 1e-8);

    // these values are dimensionless
    EXPECT_NEAR(0.003352810681183637418, GRS80::f().to<double>(), 1e-10);
    EXPECT_NEAR(298.257222101, GRS80::invf().to<double>(), 1e-10);
    EXPECT_NEAR(0.0818191910435, GRS80::e().to<double>(), 1e-10);
    EXPECT_NEAR(0.0066943800229034157495749, GRS80::e2().to<double>(), 1e-10);
}
#endif // ellipsoidTest_h__
