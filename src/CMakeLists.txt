add_library(coordinates)

add_library(coordinates::coordinates ALIAS coordinates)

target_compile_features(coordinates PUBLIC cxx_std_23)
set_target_properties(coordinates PROPERTIES
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        LINKER_LANGUAGE CXX    # the library may have no .cpp sources when GEOID/DTED are off
)

target_link_libraries(coordinates PUBLIC units::units)

coordinates_set_warnings(coordinates)

# ----------------------------
# Public headers
# ----------------------------
set(PUBLIC_HEADERS
        EGM96LUT.h
        GEOID12ALUT.h
        USGG2012LUT.h
        abstractTile.h
        abstractTileManager.h
        algorithm.h
        angles.h
        axisAccessors.h
        coordinate.h
        coordinate_traits.h
        coordinates.h
        coordinates_fwd.h
        datum.h
        dted.h
        entity.h
        dtedRawFileContents.h
        dtedTile.h
        dtedTileManager.h
        bodyFrame.h
        ellipsoid.h
        frameAxes.h
        frameOfReference.h
        hillshadeCanvas.h
        heightKinds.h
        heights.h
        kinematics.h
        latitudeConversion.h
        geodesicDirectResult.h
        geodesicInverseResult.h
        geoid.h
        helmert.h
        pose.h
        horizontalDatum.h
        intersection.h
        point.h
        ray.h
        positionAER.h
        positionECEF.h
        positionENU.h
        positionGeodetic.h
        positionNED.h
        ranges.h
        fieldOfView.h
        tileException.h
        tileMetadata.h
        topography.h
        vector.h
        vectorECEF.h
        vectorENU.h
        vectorNED.h
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
# Reusable rotation-math library headers (lib/, depends only on units)
# ----------------------------
set(LIB_HEADERS
        "${CMAKE_SOURCE_DIR}/lib/rotationDetail.h"
        "${CMAKE_SOURCE_DIR}/lib/quaternion.h"
        "${CMAKE_SOURCE_DIR}/lib/rotation.h"
        "${CMAKE_SOURCE_DIR}/lib/vector3.h"
)

target_sources(coordinates
        PUBLIC
        FILE_SET lib_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/lib"
        FILES ${LIB_HEADERS}
)

# ----------------------------
# Reusable utility headers (include/) pulled in transitively by the public headers
# (coordinates.h -> CAS.h / threadPool.h; geoid.h -> interpolate.h; the tile cache -> cache.h).
# These must be installed or a consumer's #include <coordinates.h> fails to find them.
# ----------------------------
set(UTIL_HEADERS
        "${CMAKE_SOURCE_DIR}/include/CAS.h"
        "${CMAKE_SOURCE_DIR}/include/cache.h"
        "${CMAKE_SOURCE_DIR}/include/concurrentQueue.h"
        "${CMAKE_SOURCE_DIR}/include/interpolate.h"
        "${CMAKE_SOURCE_DIR}/include/threadPool.h"
)

target_sources(coordinates
        PUBLIC
        FILE_SET util_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_SOURCE_DIR}/include"
        FILES ${UTIL_HEADERS}
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

# Line-of-sight/viewshed header (terrain-backed): a public header only when LOS is enabled, and the umbrella
# includes it under the same define so a consumer with LOS off never pulls it in.
if (COORDINATES_ENABLE_LOS)
    target_sources(coordinates PUBLIC FILE_SET public_headers TYPE HEADERS
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" FILES lineOfSight.h)
    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_LOS=1)
else()
    target_compile_definitions(coordinates PUBLIC COORDINATES_ENABLE_LOS=0)
endif()

target_sources(coordinates PRIVATE ${_sources})

# ----------------------------
# Includes
# ----------------------------
target_include_directories(coordinates
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib>
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
        FILE_SET lib_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILE_SET util_headers DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
