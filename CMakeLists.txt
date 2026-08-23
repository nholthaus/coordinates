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

# Use ccache as a compiler launcher when available, to skip re-compiling unchanged TUs. Opt-out with
# -DCOORDINATES_USE_CCACHE=OFF. `find_program` only proves a path exists, not that it can RUN in this build
# environment (a sandboxed IDE build shell may not see it) -- so actually execute `ccache --version` and only
# wire the launcher if it succeeds, otherwise skip it with a warning rather than failing every compile.
option(COORDINATES_USE_CCACHE "Use ccache as the compiler launcher when it is available and runnable" ON)
if (COORDINATES_USE_CCACHE)
	find_program(CCACHE_PROGRAM ccache)
	if (CCACHE_PROGRAM)
		execute_process(COMMAND "${CCACHE_PROGRAM}" --version
				RESULT_VARIABLE _ccache_runs
				OUTPUT_QUIET ERROR_QUIET)
		if (_ccache_runs EQUAL 0)
			set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
		endif()
		# If ccache was found but does not run in this environment, silently build without it (no warning noise).
	endif()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)
include(FetchContent)

# ----------------------------
# Options
# ----------------------------

option(COORDINATES_BUILD_TESTS     "Build coordinates tests"            ON)
option(COORDINATES_BUILD_EXAMPLES  "Build coordinates examples"         ON)
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

# Terrain line-of-sight and viewshed utilities. The terrain flavors are backed by the topography model,
# so LOS is only meaningful with DTED; default it to whatever DTED is.
option(COORDINATES_ENABLE_LOS      "Enable line-of-sight/viewshed utilities" ${COORDINATES_ENABLE_DTED})

# If you want finer grain knobs, split these:
option(COORDINATES_ENABLE_EGM96    "Enable EGM96 LUT"                   ON)
option(COORDINATES_ENABLE_GEOID12A "Enable GEOID12A LUT"                ON)
option(COORDINATES_ENABLE_USGG2012 "Enable USGG2012 LUT"                ON)

# ----------------------------
# Dependency: units
# Prefer find_package, fallback to FetchContent if requested.
# ----------------------------
find_package(units 3.6.1 CONFIG QUIET)

if (NOT units_FOUND)
	if (COORDINATES_FETCH_DEPS)
		# coordinates re-exports its own target (install(EXPORT coordinatesTargets)) and that target links
		# units::units, so units must be in an export set. From units 3.6.0 a FetchContent'd units generates no
		# install/export rules by default; opt in so units::units is exportable alongside coordinates.
		set(UNITS_INSTALL ON CACHE BOOL "" FORCE)
		FetchContent_Declare(
				units
				GIT_REPOSITORY https://github.com/nholthaus/units.git
				GIT_TAG        v3.6.1
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

# ----------------------------
# Examples
# ----------------------------
if (COORDINATES_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
