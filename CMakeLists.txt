cmake_minimum_required(VERSION 3.28)

project(coordinates
		VERSION 1.2.0
		LANGUAGES CXX
)

# ----------------------------
# Build-speed defaults
# ----------------------------
# The library uses no C++20 modules; disable the per-translation-unit module-dependency
# scan that CMake enables by default for C++20/23, which adds measurable overhead.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Use ccache as a compiler launcher when available, to skip re-compiling unchanged TUs.
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)
include(FetchContent)

# ----------------------------
# Options
# ----------------------------

option(COORDINATES_BUILD_TESTS     "Build coordinates tests"            ON)
option(COORDINATES_FETCH_DEPS      "Fetch dependencies if not found"    ON)
option(COORDINATES_WARNINGS_AS_ERRORS "Treat compiler warnings as errors on this project's own targets" OFF)

# ----------------------------
# Warning flags (applied per-target, so defaults like MSVC /EHsc are preserved and third-party
# headers such as units are not held to this project's warning level).
# ----------------------------
function(coordinates_set_warnings target)
	if (MSVC)
		target_compile_options(${target} PRIVATE /W4
				/external:anglebrackets /external:W0    # do not warn on <angle-bracket> (third-party) headers
				# Calibrate /W4: silence MSVC-only noise that is not a defect in this project's code.
				/wd4459    # local declaration hides a (library) global single-letter name (e.g. 't', 'b')
				/wd4702    # unreachable code: emitted by the optimizer inside units' constexpr pow template
				           # (a third-party header); /external:W0 cannot suppress this backend warning.
				           # Our own unreachable code is still caught by -Wall -Werror on the GCC/Clang legs.
		)
		# getenv is standard C++; MSVC deprecates it as "unsafe" (C4996). We use it deliberately.
		target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
		if (COORDINATES_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE /WX)
		endif()
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra)
		if (COORDINATES_WARNINGS_AS_ERRORS)
			target_compile_options(${target} PRIVATE -Werror)
		endif()
	endif()
endfunction()

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

# Treat the units headers as system headers so this project's strict warning level (/W4 /WX,
# -Werror) never fails on a third-party header we do not control. Applies to whichever units
# target was provided (the imported units::units, or the FetchContent-built units).
foreach(_units_target units::units units)
	if (TARGET ${_units_target})
		get_target_property(_units_aliased ${_units_target} ALIASED_TARGET)
		if (_units_aliased)
			set(_units_target ${_units_aliased})
		endif()
		get_target_property(_units_incs ${_units_target} INTERFACE_INCLUDE_DIRECTORIES)
		if (_units_incs)
			set_target_properties(${_units_target} PROPERTIES
					INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_units_incs}")
		endif()
		break()
	endif()
endforeach()

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
