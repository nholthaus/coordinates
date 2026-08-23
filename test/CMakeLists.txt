include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
)

if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

# Each test suite is its own translation unit under suites/, so they compile in parallel and
# rebuild incrementally. main.cpp provides the GoogleTest entry point; the suites register their
# tests via static initialization. (Line-of-sight suites are intentionally excluded until LOS is
# enabled.)
file(GLOB _test_suite_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/suites/*.cpp")

add_executable(coordTest
        main.cpp
        ${_test_suite_sources}
)

target_compile_features(coordTest PRIVATE cxx_std_23)
set_target_properties(coordTest PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/unitTests"
)

target_link_libraries(coordTest PRIVATE
        GTest::gtest
        coordinates::coordinates
)

coordinates_set_warnings(coordTest)

target_include_directories(coordTest PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

# resources copy
set(_test_resource_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(_test_resource_dst_dir "$<TARGET_FILE_DIR:coordTest>/resources")

add_custom_command(TARGET coordTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${_test_resource_src_dir}"
        "${_test_resource_dst_dir}"
        VERBATIM
)

include(GoogleTest)
gtest_discover_tests(coordTest
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:coordTest>"
)
