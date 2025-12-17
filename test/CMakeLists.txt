cmake_minimum_required(VERSION 3.24)

project(coordTest LANGUAGES CXX)

include(FetchContent)

# ----------------------------
# GoogleTest
# ----------------------------
FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG        v1.15.2
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