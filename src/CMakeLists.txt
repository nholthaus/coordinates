project(coordinates VERSION 0.1.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC
        EGM96LUT.h
        GEOID12ALUT.h)

target_sources(${PROJECT_NAME}
        PRIVATE
        EGM96LUT.cpp
        GEOID12ALUT.cpp
        USGG2012LUT.cpp
        INTERFACE
        FILE_SET public_headers
        TYPE HEADERS
        FILES
        abstractTile.h
        abstractTileManager.h
        algorithm.h
        coordinate_traits.h
        datum.h
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

target_compile_features(${PROJECT_NAME} PUBLIC
        cxx_std_23)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_EXTENSIONS OFF)

target_link_libraries(${PROJECT_NAME} PUBLIC
        units)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)