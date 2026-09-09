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
option(OPENGOLD_BUILD_GODOT "Build the C++ Godot tour and combat scenes" OFF)

add_library(opengold_rules STATIC src/OpenGold.Rules/src/rules.cpp src/OpenGold.Rules/src/character_rules.cpp)
target_include_directories(opengold_rules PUBLIC src/OpenGold.Rules/include)
add_library(opengold_rules_srd5 STATIC src/OpenGold.Rules.Srd5/src/srd5.cpp src/OpenGold.Rules.Srd5/src/character_rules.cpp)
target_include_directories(opengold_rules_srd5 PUBLIC src/OpenGold.Rules.Srd5/include)
target_link_libraries(opengold_rules_srd5 PUBLIC opengold_rules)

add_library(opengold_formats STATIC
    src/OpenGold.Formats/src/formats.cpp
    src/OpenGold.Formats/src/pool_radiance.cpp
    src/OpenGold.Formats/src/geo_map.cpp
    src/OpenGold.Formats/src/ecl.cpp
    src/OpenGold.Formats/src/wall_art.cpp
    src/OpenGold.Formats/src/por_sound.cpp
)

target_include_directories(opengold_formats
    PUBLIC
        src/OpenGold.Formats/include
)

add_library(opengold_core STATIC
    src/OpenGold.Core/src/core.cpp
    src/OpenGold.Core/src/character_art.cpp
    src/OpenGold.Core/src/character_creator.cpp
    src/OpenGold.Core/src/character_pool.cpp
    src/OpenGold.Core/src/character.cpp
    src/OpenGold.Core/src/campaign_party.cpp
    src/OpenGold.Core/src/campaign_save.cpp
    src/OpenGold.Core/src/inventory.cpp
    src/OpenGold.Core/src/creature_catalog.cpp
    src/OpenGold.Core/src/creature_factory.cpp
    src/OpenGold.Core/src/map_catalog.cpp
    src/OpenGold.Core/src/ecl_machine.cpp
    src/OpenGold.Core/src/por_effects.cpp
    src/OpenGold.Core/src/rolf_tour.cpp
    src/OpenGold.Core/src/phlan_session.cpp
    src/OpenGold.Core/src/combat_demo.cpp
    src/OpenGold.Core/src/exploration_view.cpp
    src/OpenGold.Core/src/speaker_audio.cpp
    src/OpenGold.Core/src/sound_bank.cpp
    src/OpenGold.Core/src/sound_player.cpp
)

target_include_directories(opengold_core
    PUBLIC
        src/OpenGold.Core/include
)

target_link_libraries(opengold_core
    PUBLIC
        opengold_formats
        opengold_rules
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
    add_executable(opengold_rules_tests tests/rules_tests.cpp)
    target_link_libraries(opengold_rules_tests PRIVATE opengold_rules_srd5 opengold_core)
    target_compile_definitions(opengold_rules_tests PRIVATE OPENGOLD_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME opengold_rules_tests COMMAND opengold_rules_tests)
    add_executable(opengold_character_tests tests/character_tests.cpp)
    target_link_libraries(opengold_character_tests PRIVATE opengold_core opengold_rules_srd5)
    add_test(NAME opengold_character_tests COMMAND opengold_character_tests)
    add_executable(opengold_party_tests tests/party_tests.cpp)
    target_link_libraries(opengold_party_tests PRIVATE opengold_core opengold_rules_srd5)
    target_compile_definitions(opengold_party_tests PRIVATE OPENGOLD_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME opengold_party_tests COMMAND opengold_party_tests)
    add_executable(opengold_save_tests tests/save_tests.cpp)
    target_link_libraries(opengold_save_tests PRIVATE opengold_core opengold_rules_srd5)
    target_compile_definitions(opengold_save_tests PRIVATE OPENGOLD_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME opengold_save_tests COMMAND opengold_save_tests)
    add_executable(opengold_sound_tests tests/sound_tests.cpp)
    target_link_libraries(opengold_sound_tests PRIVATE opengold_core)
    add_test(NAME opengold_sound_tests COMMAND opengold_sound_tests)
    add_executable(opengold_sound_player_tests tests/sound_player_tests.cpp)
    target_link_libraries(opengold_sound_player_tests PRIVATE opengold_core)
    add_test(NAME opengold_sound_player_tests COMMAND opengold_sound_player_tests)
endif()

add_executable(opengold_creatures tools/creatures.cpp)
target_link_libraries(opengold_creatures PRIVATE opengold_core)
add_executable(opengold_maps tools/maps.cpp)
target_link_libraries(opengold_maps PRIVATE opengold_core)
add_executable(opengold_scripts tools/scripts.cpp)
target_link_libraries(opengold_scripts PRIVATE opengold_core)

if(OPENGOLD_BUILD_GODOT)
    set_target_properties(opengold_core opengold_formats opengold_rules opengold_rules_srd5 PROPERTIES POSITION_INDEPENDENT_CODE ON)
    add_subdirectory(src/OpenGold.Godot)
endif()
