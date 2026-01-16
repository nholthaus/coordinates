//--------------------------------------------------------------------------------------------------
//
//	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
//
//--------------------------------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//--------------------------------------------------------------------------------------------------
//
// Copyright (c) 2016 Nic Holthaus
//
//--------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gtest_units.h>

#include "cacheTest.h"
 
#include "ellipsoidTest.h"
#include "geoidTest.h"

#include "helmertTest.h"
#include "topographyTest.h"
#include "horizontalDatumTest.h"
#include "verticalDatumTest.h"
#include "datumTest.h"

#include "frameOfReferenceTest.h"
#include "pointTest.h"
#include "positionECEFTest.h"
#include "positionGeodeticTest.h"
#include "positionENUTest.h"
#include "positionNEDTest.h"
#include "positionAERTest.h"

#include "geodesicTest.h"
#include "intersectionTest.h"
#include "losTest.h"

int main(int argc, char* argv[])
{
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}