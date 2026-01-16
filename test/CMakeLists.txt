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

add_executable(coordTest
        main.cpp
)

target_compile_features(coordTest PRIVATE cxx_std_23)
set_target_properties(coordTest PROPERTIES
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/unitTests"
)

target_link_libraries(coordTest PRIVATE
        GTest::gtest_main
        coordinates::coordinates
)

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
