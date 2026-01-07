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

cmake_minimum_required(VERSION 3.28)

project(coord VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

# ----------------------------
# Dependencies
# ----------------------------
include(FetchContent)

# Units v3 (nholthaus/units) — adjust tag if you want a specific release
fetchcontent_declare(
		units
		GIT_REPOSITORY https://github.com/nholthaus/units.git
		GIT_TAG v3.3.0
		)
fetchcontent_makeavailable(units)

# ----------------------------
# LIBRARY (HEADER-ONLY)
# ----------------------------
add_library(${PROJECT_NAME} INTERFACE)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(${PROJECT_NAME} INTERFACE units)
target_include_directories(${PROJECT_NAME} INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )

# ----------------------------
# Tests
# ----------------------------
if (BUILD_TESTING)
	add_subdirectory(test)
endif ()