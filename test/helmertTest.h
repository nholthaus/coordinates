#ifndef helmertTest_h__
#define helmertTest_h__

//------------------------
//	INCLUDES
//------------------------

#include <coord/horizontalDatum.h>

#include <stdexcept>

namespace
{
// The fixture for testing
class HelmertTest : public ::testing::Test {
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    HelmertTest()
    {
        // You can do set-up work for each test here.
    }

    virtual ~HelmertTest()
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

    // Coefficients from example in EPSG guidance 7.2 section 2.4.4.1
    struct EPSG_GUIDANCE_7_2_SEC_24321
    {
        /// X-axis translation
        static constexpr auto tx() -> meters<>
        {
            return 0.000_m;
        }

        /// Y-axis translation
        static constexpr auto ty() -> meters<>
        {
            return 0.000_m;
        }

        /// Z-axis translation
        static constexpr auto tz() -> meters<>
        {
            return 4.500_m;
        }

        /// Rotation about the X-axis
        static constexpr auto rx() -> arcseconds<>
        {
            return arcseconds(0.000);
        }

        /// Rotation about the Y-axis
        static constexpr auto ry() -> arcseconds<>
        {
            return arcseconds(0.000);
        }

        /// Rotation about the Z-axis
        static constexpr auto rz() -> arcseconds<>
        {
            return arcseconds(0.554);
        }

        /// Scale factor
        static constexpr auto s() -> parts_per_million<>
        {
            return parts_per_million(0.219);
        }
    };

    struct notHelmert {};
};

TEST_F(HelmertTest, hasTx)
{
    EXPECT_TRUE(coord::traits::has_tx<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_tx<horizontalDatums::NAD83>::value);
    EXPECT_TRUE(coord::traits::has_tx<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_tx<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_tx<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_tx<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasTy)
{
    EXPECT_TRUE(coord::traits::has_ty<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_ty<horizontalDatums::NAD83>::value);
    EXPECT_TRUE(coord::traits::has_ty<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_ty<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_ty<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_ty<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasTz)
{
    EXPECT_TRUE( coord::traits::has_tz<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_tz<horizontalDatums::NAD83>::value);
    EXPECT_TRUE( coord::traits::has_tz<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_tz<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_tz<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_tz<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasS)
{
    EXPECT_TRUE( coord::traits::has_s<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_s<horizontalDatums::NAD83>::value);
    EXPECT_TRUE( coord::traits::has_s<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_s<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_s<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_s<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasRx)
{
    EXPECT_TRUE( coord::traits::has_rx<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_rx<horizontalDatums::NAD83>::value);
    EXPECT_TRUE( coord::traits::has_rx<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_rx<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_rx<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_rx<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasRy)
{
    EXPECT_TRUE( coord::traits::has_ry<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_ry<horizontalDatums::NAD83>::value);
    EXPECT_TRUE( coord::traits::has_ry<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_ry<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_ry<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_ry<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasRz)
{
    EXPECT_TRUE( coord::traits::has_rz<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::has_rz<horizontalDatums::NAD83>::value);
    EXPECT_TRUE( coord::traits::has_rz<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_rz<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_rz<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_rz<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, isHelmert7param)
{
    EXPECT_TRUE(coord::traits::is_helmert_7param_coefficients<horizontalDatums::WGS84_G1674>::value);
	EXPECT_TRUE(coord::traits::is_helmert_7param_coefficients<horizontalDatums::NAD83>::value);
    EXPECT_TRUE(coord::traits::is_helmert_7param_coefficients<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::is_helmert_7param_coefficients<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::is_helmert_7param_coefficients<notHelmert>::value);
	EXPECT_TRUE(coord::traits::is_helmert_7param_coefficients<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDtx)
{
    EXPECT_TRUE(coord::traits::has_dtx<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_dtx<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_dtx<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_dtx<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_dtx<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_dtx<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDty)
{
    EXPECT_TRUE(coord::traits::has_dty<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_dty<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_dty<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_dty<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_dty<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_dty<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDtz)
{
    EXPECT_TRUE(coord::traits::has_dtz<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_dtz<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_dtz<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_dtz<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_dtz<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_dtz<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDs)
{
    EXPECT_TRUE(coord::traits::has_ds<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_ds<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_ds<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_ds<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_ds<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_ds<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDrx)
{
    EXPECT_TRUE(coord::traits::has_drx<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_drx<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_drx<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_drx<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_drx<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_drx<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDry)
{
    EXPECT_TRUE(coord::traits::has_dry<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_dry<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_dry<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_dry<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_dry<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_dry<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasDrz)
{
    EXPECT_TRUE(coord::traits::has_drz<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_drz<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_drz<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_drz<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_drz<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_drz<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, hasEpoch)
{
    EXPECT_TRUE(coord::traits::has_epoch<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::has_epoch<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::has_epoch<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::has_epoch<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::has_epoch<notHelmert>::value);
	EXPECT_TRUE(coord::traits::has_epoch<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, isHelmert14param)
{
    EXPECT_TRUE(coord::traits::is_helmert_14param_coefficients<horizontalDatums::WGS84_G1674>::value);
    EXPECT_TRUE(coord::traits::is_helmert_14param_coefficients<horizontalDatums::NAD83>::value);
    EXPECT_FALSE(coord::traits::is_helmert_14param_coefficients<EPSG_GUIDANCE_7_2_SEC_24321>::value);
    EXPECT_TRUE(coord::traits::is_helmert_14param_coefficients<horizontalDatums::GDA94>::value);
    EXPECT_FALSE(coord::traits::is_helmert_14param_coefficients<notHelmert>::value);
    EXPECT_TRUE(coord::traits::is_helmert_14param_coefficients<inverse_coefficients<horizontalDatums::GDA94>>::value);
}

TEST_F(HelmertTest, inverse)
{
    using inv = coord::inverse_coefficients<horizontalDatums::NAD83>;

    EXPECT_NEAR(-0.99343, inv::tx().to<double>(), 1e-6);
	EXPECT_NEAR(1.90331, inv::ty().to<double>(), 1e-6);
	EXPECT_NEAR(0.52655, inv::tz().to<double>(), 1e-6);
	EXPECT_NEAR(-1.71504_ppb, inv::s(), 1e-6_ppb);
	EXPECT_NEAR(25.91467, inv::rx().to<double>(), 1e-6);
	EXPECT_NEAR(9.42645, inv::ry().to<double>(), 1e-6);
	EXPECT_NEAR(11.59935, inv::rz().to<double>(), 1e-6);
}

TEST_F(HelmertTest, 7ParamTransform)
{
    // Source "EPSG Guidance 7.2, Section 2.4.3.2.1" example
    using m = units::length::meters;
	using ppms = units::concentration::ppms;

    auto wgs72 = std::make_tuple(m(3657660.66), m(255768.55), m(5201382.11));
    auto wgs84 = coord::positionVectorTransform<EPSG_GUIDANCE_7_2_SEC_24321>(wgs72);

    EXPECT_NEAR(0.0, meters(EPSG_GUIDANCE_7_2_SEC_24321::tx()).to<double>(), 5.0e-13);
    EXPECT_NEAR(0.0, meters(EPSG_GUIDANCE_7_2_SEC_24321::ty()).to<double>(), 5.0e-13);
    EXPECT_NEAR(4.5, meters(EPSG_GUIDANCE_7_2_SEC_24321::tz()).to<double>(), 5.0e-13);
    EXPECT_NEAR(0.000000000000, radians(EPSG_GUIDANCE_7_2_SEC_24321::rx()).to<double>(), 5.0e-13);
    EXPECT_NEAR(0.000000000000, radians(EPSG_GUIDANCE_7_2_SEC_24321::ry()).to<double>(), 5.0e-13);
    EXPECT_NEAR(0.000002685868, radians(EPSG_GUIDANCE_7_2_SEC_24321::rz()).to<double>(), 5.0e-13);
    EXPECT_NEAR(0.000000219, EPSG_GUIDANCE_7_2_SEC_24321::s().to<double>(), 5.0e-13);

    // 1 cm accuracy
    EXPECT_NEAR(3657660.78, std::get<0>(wgs84).to<double>(), 1.0e-2);
    EXPECT_NEAR(255778.43, std::get<1>(wgs84).to<double>(), 1.0e-2);
    EXPECT_NEAR(5201387.75, std::get<2>(wgs84).to<double>(), 1.0e-2);

    auto wgs72result = coord::inversePositionVectorTransform<EPSG_GUIDANCE_7_2_SEC_24321>(wgs84);

    EXPECT_NEAR(m(std::get<0>(wgs72)).to<double>(), m(std::get<0>(wgs72result)).to<double>(), 5.0e-4);
    EXPECT_NEAR(m(std::get<1>(wgs72)).to<double>(), m(std::get<1>(wgs72result)).to<double>(), 5.0e-4);
    EXPECT_NEAR(m(std::get<2>(wgs72)).to<double>(), m(std::get<2>(wgs72result)).to<double>(), 5.0e-4);
}

TEST_F(HelmertTest, 14ParamTransform)
{
    // Source "EPSG Guidance 7.2, Section 2.4.3.4" example
    using m = units::length::meters;
    using rad = units::angle::radians;
    using ppm = units::concentration::ppms;

    auto itrf2008 = std::make_tuple(m(-3789470.710), m(4841770.404), m(-1690893.952));
    auto gda94 = coord::positionVectorTransform<horizontalDatums::GDA94>(itrf2008, 2013.90_yr);

	EXPECT_NEAR(-0.056, m(horizontalDatums::GDA94::tx() + horizontalDatums::GDA94::dtx()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-4);
	EXPECT_NEAR(0.007, m(horizontalDatums::GDA94::ty() + horizontalDatums::GDA94::dty()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-4);
	EXPECT_NEAR(0.050, m(horizontalDatums::GDA94::tz() + horizontalDatums::GDA94::dtz()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-4);
	EXPECT_NEAR(-1.471021e-07, rad(horizontalDatums::GDA94::rx() + horizontalDatums::GDA94::drx()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-12);
	EXPECT_NEAR(-1.249830e-07, rad(horizontalDatums::GDA94::ry() + horizontalDatums::GDA94::dry()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-12);
	EXPECT_NEAR(-1.230844e-07, rad(horizontalDatums::GDA94::rz() + horizontalDatums::GDA94::drz()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-12);
	EXPECT_NEAR(0.00000001188, ppm(horizontalDatums::GDA94::s() + horizontalDatums::GDA94::ds()*(2013.9_yr - horizontalDatums::GDA94::epoch())).to<double>(), 5.0e-5);

    EXPECT_NEAR(-3789470.004, m(std::get<0>(gda94)).to<double>(), 5.0e-4);
    EXPECT_NEAR( 4841770.686, m(std::get<1>(gda94)).to<double>(), 5.0e-4);
    EXPECT_NEAR(-1690895.108, m(std::get<2>(gda94)).to<double>(), 5.0e-4);

	auto itrf2008result = coord::inversePositionVectorTransform<horizontalDatums::GDA94>(gda94, 2013.90_yr);

    EXPECT_NEAR(m(std::get<0>(itrf2008)).to<double>(), m(std::get<0>(itrf2008result)).to<double>(), 5.0e-4);
    EXPECT_NEAR(m(std::get<1>(itrf2008)).to<double>(), m(std::get<1>(itrf2008result)).to<double>(), 5.0e-4);
    EXPECT_NEAR(m(std::get<2>(itrf2008)).to<double>(), m(std::get<2>(itrf2008result)).to<double>(), 5.0e-4);

	// source: http://webapp.geod.nrcan.gc.ca/geod/tools-outils/trx.php?locale=en
	// with epoch transformation
	std::tuple<meters, meters, meters> itrs2008_2014(1532138.956_m, -4464558.719_m, 4275244.397_m);
	std::tuple<meters, meters, meters> itrf2000_2014_expected(1532138.959_m, -4464558.730_m, 4275244.372_m);
	auto itrf2000_2014_result = coord::positionVectorTransform<horizontalDatums::ITRF2000>(itrs2008_2014, 2014.0_yr);

	EXPECT_NEAR(std::get<0>(itrf2000_2014_expected).to<double>(), std::get<0>(itrf2000_2014_result).to<double>(), 5.0e-4);
	EXPECT_NEAR(std::get<1>(itrf2000_2014_expected).to<double>(), std::get<1>(itrf2000_2014_result).to<double>(), 5.0e-4);
	EXPECT_NEAR(std::get<2>(itrf2000_2014_expected).to<double>(), std::get<2>(itrf2000_2014_result).to<double>(), 5.0e-4);
}
}

#endif // helmertTest_h__