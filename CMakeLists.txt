cmake_minimum_required(VERSION 3.24)

project(OpenGold VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    # Keep core, tests and the Godot boundary on the same CRT/iterator ABI.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

option(OPENGOLD_BUILD_TESTS "Build the native test suite" ${BUILD_TESTING})
option(OPENGOLD_BUILD_GODOT "Build the C++ Godot tour scene" OFF)

add_library(opengold_formats STATIC
    src/OpenGold.Formats/src/formats.cpp
    src/OpenGold.Formats/src/pool_radiance.cpp
    src/OpenGold.Formats/src/geo_map.cpp
    src/OpenGold.Formats/src/ecl.cpp
)

target_include_directories(opengold_formats
    PUBLIC
        src/OpenGold.Formats/include
)

add_library(opengold_core STATIC
    src/OpenGold.Core/src/core.cpp
    src/OpenGold.Core/src/creature_catalog.cpp
    src/OpenGold.Core/src/creature_factory.cpp
    src/OpenGold.Core/src/map_catalog.cpp
    src/OpenGold.Core/src/ecl_machine.cpp
    src/OpenGold.Core/src/por_effects.cpp
    src/OpenGold.Core/src/rolf_tour.cpp
)

target_include_directories(opengold_core
    PUBLIC
        src/OpenGold.Core/include
)

target_link_libraries(opengold_core
    PUBLIC
        opengold_formats
)

if(OPENGOLD_BUILD_TESTS)
    enable_testing()
    add_executable(opengold_core_tests
        tests/core_tests.cpp
    )
    target_link_libraries(opengold_core_tests PRIVATE opengold_core)
    add_test(NAME opengold_core_tests COMMAND opengold_core_tests)
    add_executable(opengold_creature_tests tests/creature_tests.cpp)
    target_link_libraries(opengold_creature_tests PRIVATE opengold_core)
    add_test(NAME opengold_creature_tests COMMAND opengold_creature_tests)
    add_executable(opengold_ecl_tests tests/ecl_tests.cpp)
    target_link_libraries(opengold_ecl_tests PRIVATE opengold_core)
    add_test(NAME opengold_ecl_tests COMMAND opengold_ecl_tests)
    add_executable(opengold_tour_tests tests/tour_tests.cpp)
    target_link_libraries(opengold_tour_tests PRIVATE opengold_core)
    add_test(NAME opengold_tour_tests COMMAND opengold_tour_tests)
endif()

add_executable(opengold_creatures tools/creatures.cpp)
target_link_libraries(opengold_creatures PRIVATE opengold_core)
add_executable(opengold_maps tools/maps.cpp)
target_link_libraries(opengold_maps PRIVATE opengold_core)
add_executable(opengold_scripts tools/scripts.cpp)
target_link_libraries(opengold_scripts PRIVATE opengold_core)

if(OPENGOLD_BUILD_GODOT)
    set_target_properties(opengold_core opengold_formats PROPERTIES POSITION_INDEPENDENT_CODE ON)
    add_subdirectory(src/OpenGold.Godot)
endif()
