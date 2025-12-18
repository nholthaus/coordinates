cmake_minimum_required(VERSION 3.24)

project(coord VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

# ----------------------------
# Library: pcCoord (header-only)
# ----------------------------
add_library(coord INTERFACE)

target_compile_features(coord INTERFACE cxx_std_23)
set_target_properties(coord PROPERTIES
    CXX_EXTENSIONS OFF
)

target_include_directories(coord
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ----------------------------
# Dependencies
# ----------------------------
include(FetchContent)

# Units v3 (nholthaus/units) — adjust tag if you want a specific release
FetchContent_Declare(
    units
    GIT_REPOSITORY https://github.com/nholthaus/units.git
    GIT_TAG        v3.2.0
)
FetchContent_MakeAvailable(units)

# Expose units to consumers of pcCoord
# (Exact target name depends on units' exported targets; common is "units" or "nholthaus::units")
# We'll try the simplest first; if it differs, I’ll adapt once you show the units config you prefer.
target_link_libraries(coord INTERFACE units)

# ----------------------------
# Tests
# ----------------------------
if(BUILD_TESTING)
    add_subdirectory(test)
endif()