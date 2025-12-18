#include <gtest/gtest.h>
#include <gtest_units.h>
//#include <pcCoord.h>
 
#include "ellipsoidTest.h"
#include "geoidTest.h"
//
//#include "helmertTest.h"
//#include "topographyTest.h"
//#include "horizontalDatumTest.h"
//#include "verticalDatumTest.h"
//#include "datumTest.h"
//
//#include "frameOfReferenceTest.h"
//#include "pointTest.h"
//#include "positionECEFTest.h"
//#include "positionGeodeticTest.h"
//#include "positionENUTest.h"
//#include "positionNEDTest.h"
//#include "positionAERTest.h"

int main(int argc, char* argv[])
{
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}