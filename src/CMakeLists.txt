add_library(coordinates)

add_library(coordinates::coordinates ALIAS coordinates)

target_compile_features(coordinates PUBLIC cxx_std_23)
set_target_properties(coordinates PROPERTIES
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(coordinates PUBLIC units)

# ----------------------------
# Public headers
# ----------------------------
set(PUBLIC_HEADERS
        EGM96LUT.h
        GEOID12ALUT.h
        abstractTile.h
        abstractTileManager.h
        algorithm.h
        coordinate_traits.h
        datum.h
        dted.h
        dtedRawFileContents.h
        dtedTile.h
        dtedTileManager.h
        ellipsoid.h
        frameOfReference.h
        geoid.h
        helmert.h
        horizontalDatum.h
        point.h
        positionAER.h
        positionECEF.h
        positionENU.h
        positionGeodetic.h
        positionNED.h
        positionXYZ.h
        tileMetadata.h
        topography.h
        verticalDatum.h
)

# If some headers are only meaningful with DTED/GEOID, you can optionally
# prune _public_headers here too — but don’t break includes unexpectedly.
target_sources(coordinates
        PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES ${PUBLIC_HEADERS}
)

# ----------------------------
# Sources with bloat knobs
# ----------------------------
set(_sources)

if (COORDINATES_ENABLE_GEOIDS)
    if (COORDINATES_ENABLE_EGM96)
        list(APPEND _sources EGM96LUT.cpp)
        target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_EGM96=1)
    endif()

    if (COORDINATES_ENABLE_GEOID12A)
        list(APPEND _sources GEOID12ALUT.cpp)
        target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_GEOID12A=1)
    endif()

    if (COORDINATES_ENABLE_USGG2012)
        list(APPEND _sources USGG2012LUT.cpp)
        target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_USGG2012=1)
    endif()

    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_GEOIDS=1)
else()
    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_GEOIDS=0)
endif()

if (COORDINATES_ENABLE_DTED)
    list(APPEND _sources
            dtedTile.cpp
            dtedTileManager.cpp
    )
    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_DTED=1)
else()
    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_DTED=0)
endif()

target_sources(coordinates PRIVATE ${_sources})

# ----------------------------
# Includes
# ----------------------------
target_include_directories(coordinates
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ----------------------------
# Install
# ----------------------------
install(TARGETS coordinates
        EXPORT coordinatesTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        FILE_SET public_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
