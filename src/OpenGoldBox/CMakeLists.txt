if(NOT WIN32)
    message(FATAL_ERROR "The OpenGoldBox executable target currently supports Windows only.")
endif()
include(FetchContent)
if(NOT Python3_EXECUTABLE AND EXISTS "${PROJECT_SOURCE_DIR}/build/_deps/python/python.exe")
    set(Python3_EXECUTABLE "${PROJECT_SOURCE_DIR}/build/_deps/python/python.exe")
endif()
find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE PATH "" FORCE)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${GODOTCPP_BUILD_PROFILE}")
set(GODOTCPP_DISABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
set(GODOTCPP_USE_HOT_RELOAD OFF CACHE BOOL "" FORCE)
FetchContent_Declare(godot_cpp
    GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
    GIT_TAG e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77)
FetchContent_MakeAvailable(godot_cpp)
find_program(OPENGOLDBOX_GODOT NAMES godot_console godot REQUIRED)
set(OPENGOLDBOX_DEBUG_TEMPLATE "${PROJECT_SOURCE_DIR}/build/export-templates/windows_debug_x86_64.exe"
    CACHE FILEPATH "Godot Windows x64 debug export template matching the installed editor")
if(NOT EXISTS "${OPENGOLDBOX_DEBUG_TEMPLATE}")
    message(STATUS "Executable export needs OPENGOLDBOX_DEBUG_TEMPLATE. The game DLL can be built without templates; see src/OpenGoldBox/README.md.")
endif()
file(TO_CMAKE_PATH "${OPENGOLDBOX_DEBUG_TEMPLATE}" OPENGOLDBOX_DEBUG_TEMPLATE)
set(game_project "${CMAKE_CURRENT_SOURCE_DIR}/godot")
set(game_output "${PROJECT_SOURCE_DIR}/build/OpenGoldBox")
configure_file(export_presets.cfg.in "${game_project}/export_presets.cfg" @ONLY)

add_library(opengoldbox_game SHARED rolf_tour_view.cpp combat_view.cpp character_creation_view.cpp
    portrait_picker.cpp party_preview.cpp level_up_view.cpp campaign_save_view.cpp save_slots.cpp
    character_pool_view.cpp character_sheet_view.cpp godot_sound_output.cpp register_types.cpp)
target_link_libraries(opengoldbox_game PRIVATE opengold_core opengold_rules_srd5 godot-cpp)
target_compile_features(opengoldbox_game PRIVATE cxx_std_20)
target_compile_definitions(opengoldbox_game PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
set_target_properties(opengoldbox_game PROPERTIES PREFIX "" RUNTIME_OUTPUT_DIRECTORY "${game_project}/bin")

file(GLOB portrait_sources CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/art/portraits/*.png"
    "${PROJECT_SOURCE_DIR}/data/art/portraits/*.png" "${PROJECT_SOURCE_DIR}/art/portraits/portraits.json")
set(portrait_outputs)
foreach(source IN LISTS portrait_sources)
    get_filename_component(name "${source}" NAME)
    set(output "${game_project}/bin/portraits/${name}")
    add_custom_command(OUTPUT "${output}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${game_project}/bin/portraits"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${source}" "${output}"
        DEPENDS "${source}" VERBATIM)
    list(APPEND portrait_outputs "${output}")
endforeach()
add_custom_target(opengoldbox_assets DEPENDS ${portrait_outputs})
add_dependencies(opengoldbox_game opengoldbox_assets)

# Godot exports the executable; the C++ game code is its GDExtension.
# Rules remain loose files because the engine-independent loader uses filesystem I/O.
add_custom_target(OpenGoldBox ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/opengold.gdextension" "${game_project}/bin/opengold.gdextension"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data/rules/srd-5.2.1" "${game_project}/data/rules/srd-5.2.1"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${game_output}"
    COMMAND "${OPENGOLDBOX_GODOT}" --headless --editor --path "${game_project}" --import --quit
    COMMAND "${OPENGOLDBOX_GODOT}" --headless --path "${game_project}" --export-debug "Windows Desktop" "${game_output}/opengoldbox.exe"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data/rules/srd-5.2.1" "${game_output}/data/rules/srd-5.2.1"
    DEPENDS opengoldbox_game opengoldbox_assets
    VERBATIM)
