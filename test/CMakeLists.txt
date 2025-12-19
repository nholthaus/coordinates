# --------------------------------------------------------------------------------------------------
#
# 	Coordinates: A compile-time c++23 coordinate conversion library based on `units`
#
# --------------------------------------------------------------------------------------------------
#
#  The MIT License (MIT)
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
#  and associated documentation files (the "Software"), to deal in the Software without
#  restriction, including without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all copies or
#  substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
#  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# --------------------------------------------------------------------------------------------------
#
#  Copyright (c) 2016 Nic Holthaus
#
# --------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.24)

project(coordTest LANGUAGES CXX)

include(FetchContent)

# ----------------------------
# GoogleTest
# ----------------------------
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # good default on MSVC
FetchContent_MakeAvailable(googletest)

# ----------------------------
# Target
# ----------------------------
add_executable(coordTest
        main.cpp

        # Keeping headers listed is optional; helps IDE browsing
        ellipsoidTest.h
        geoidTest.h
        frameOfReferenceTest.h
        pointTest.h
        helmertTest.h
        horizontalDatumTest.h
        verticalDatumTest.h
        datumTest.h
        topographyTest.h
        positionECEFTest.h
        positionGeodeticTest.h
        positionENUTest.h
        positionNEDTest.h
        positionAERTest.h
        gtest_units.h
)

target_compile_features(coordTest PRIVATE cxx_std_23)
set_target_properties(coordTest PROPERTIES
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/unitTests"
)

target_link_libraries(coordTest
        PRIVATE
        GTest::gtest_main
        coord
)
target_include_directories(coordTest
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ----------------------------
# Test resources (no Qt)
# ----------------------------
set(_test_resource_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(_test_resource_dst_dir "$<TARGET_FILE_DIR:coordTest>/resources")

add_custom_command(TARGET coordTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_test_resource_src_dir}"
        "${_test_resource_dst_dir}"
        VERBATIM
)

# ----------------------------
# CTest registration
# ----------------------------
include(GoogleTest)
gtest_discover_tests(coordTest
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:coordTest>"
)