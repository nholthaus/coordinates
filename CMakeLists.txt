cmake_minimum_required(VERSION 3.28)

project(coordinates
		VERSION 0.1.0
		LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)
include(FetchContent)

# ----------------------------
# Options
# ----------------------------

option(COORDINATES_BUILD_TESTS     "Build coordinates tests"            ON)
option(COORDINATES_FETCH_DEPS      "Fetch dependencies if not found"    ON)

option(COORDINATES_ENABLE_DTED     "Enable DTED/topography support"     ON)
option(COORDINATES_ENABLE_GEOIDS   "Enable geoid/LUT support"           ON)

# If you want finer grain knobs, split these:
option(COORDINATES_ENABLE_EGM96    "Enable EGM96 LUT"                   ON)
option(COORDINATES_ENABLE_GEOID12A "Enable GEOID12A LUT"                ON)
option(COORDINATES_ENABLE_USGG2012 "Enable USGG2012 LUT"                ON)

# ----------------------------
# Dependency: units
# Prefer find_package, fallback to FetchContent if requested.
# ----------------------------
find_package(units 3.5.1 CONFIG QUIET)

if (NOT units_FOUND)
	if (COORDINATES_FETCH_DEPS)
		FetchContent_Declare(
				units
				GIT_REPOSITORY https://github.com/nholthaus/units.git
				GIT_TAG        v3.5.1
		)
		FetchContent_MakeAvailable(units)
	else()
		message(FATAL_ERROR
				"units not found. Install units (CONFIG package) or set COORDINATES_FETCH_DEPS=ON."
		)
	endif()
endif()

add_subdirectory(src)

# ----------------------------
# Package + install
# ----------------------------
# Export targets from the build tree for local consumption too
export(
		EXPORT coordinatesTargets
		NAMESPACE coordinates::
		FILE "${CMAKE_CURRENT_BINARY_DIR}/coordinatesTargets.cmake"
)

configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/coordinatesConfig.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/coordinatesConfig.cmake"
		INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/coordinates"
)

write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/coordinatesConfigVersion.cmake"
		VERSION "${PROJECT_VERSION}"
		COMPATIBILITY SameMajorVersion
)

install(
		EXPORT coordinatesTargets
		NAMESPACE coordinates::
		FILE coordinatesTargets.cmake
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/coordinates"
)

install(
		FILES
		"${CMAKE_CURRENT_BINARY_DIR}/coordinatesConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/coordinatesConfigVersion.cmake"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/coordinates"
)

# ----------------------------
# Tests
# ----------------------------
if (COORDINATES_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
