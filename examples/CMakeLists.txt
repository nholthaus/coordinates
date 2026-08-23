# ----------------------------
# Examples
# ----------------------------
# End-to-end programs that exercise the library as a whole. They are built only when examples are enabled
# AND the features they need are present (the sensor/terrain and tower/horizon demos need DTED topography).

if (NOT COORDINATES_ENABLE_DTED)
	message(STATUS "coordinates: examples skipped (COORDINATES_ENABLE_DTED is OFF)")
	return()
endif()

# The DTED tile both examples read at runtime (shared with the test suite's resources). Copy it beside each
# executable so the demo runs straight from the build tree.
set(_example_tile "${CMAKE_SOURCE_DIR}/test/resources/w115_n37.dt2")

# Build one example target from a single source file: C++23, links the library, this project's warning level,
# and the runtime DTED tile staged next to the binary.
function(coordinates_add_example name)
	add_executable(${name} ${name}.cpp)
	target_compile_features(${name} PRIVATE cxx_std_26)
	target_link_libraries(${name} PRIVATE coordinates::coordinates)
	coordinates_set_warnings(${name})
	if (EXISTS "${_example_tile}")
		add_custom_command(TARGET ${name} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${name}>/test/resources"
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${_example_tile}"
				"$<TARGET_FILE_DIR:${name}>/test/resources/w115_n37.dt2"
				VERBATIM)
	endif()
endfunction()

coordinates_add_example(sensorTerrainHit)
coordinates_add_example(towerHorizonScan)
coordinates_add_example(f35Attitude)
