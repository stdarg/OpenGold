if(NOT WIN32 AND NOT APPLE)
    message(FATAL_ERROR "The OpenGoldBox executable target supports Windows and macOS only.")
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
find_program(OPENGOLDBOX_GODOT NAMES godot_console godot Godot
    HINTS "/Applications/Godot.app/Contents/MacOS" "/Applications/Godot_mono.app/Contents/MacOS" REQUIRED)
set(game_project "${CMAKE_CURRENT_SOURCE_DIR}/godot")
if(WIN32)
    set(OPENGOLDBOX_DEBUG_TEMPLATE "${PROJECT_SOURCE_DIR}/build/export-templates/windows_debug_x86_64.exe"
        CACHE FILEPATH "Godot Windows x64 debug export template matching the installed editor")
    if(NOT EXISTS "${OPENGOLDBOX_DEBUG_TEMPLATE}")
        message(STATUS "Executable export needs OPENGOLDBOX_DEBUG_TEMPLATE. The game DLL can be built without templates; see src/OpenGoldBox/README.md.")
    endif()
    file(TO_CMAKE_PATH "${OPENGOLDBOX_DEBUG_TEMPLATE}" OPENGOLDBOX_DEBUG_TEMPLATE)
    set(game_output "${PROJECT_SOURCE_DIR}/win-package")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
    if(NOT CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        message(FATAL_ERROR "Visual C++ runtime DLLs were not found for the Windows package.")
    endif()
    set(export_preset "Windows Desktop")
    set(export_path "${game_output}/opengoldbox.exe")
    set(export_config "export_presets.cfg.in")
else()
    set(game_output "${PROJECT_SOURCE_DIR}/mac-package")
    set(export_preset "macOS")
    set(export_path "${game_output}/OpenGoldBox.app")
    set(export_config "export_presets.macos.cfg.in")
endif()
configure_file("${export_config}" "${game_project}/export_presets.cfg" @ONLY)

add_library(opengoldbox_game SHARED rolf_tour_view.cpp combat_view.cpp character_creation_view.cpp
    portrait_picker.cpp party_preview.cpp level_up_view.cpp campaign_save_view.cpp save_slots.cpp
    character_pool_view.cpp character_sheet_view.cpp godot_sound_output.cpp register_types.cpp startup_view.cpp game_lifecycle.cpp localization.cpp application_settings.cpp)
target_link_libraries(opengoldbox_game PRIVATE opengold_core opengold_rules_srd5 godot-cpp)
target_compile_features(opengoldbox_game PRIVATE cxx_std_20)
target_compile_definitions(opengoldbox_game PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
set_target_properties(opengoldbox_game PROPERTIES PREFIX "" RUNTIME_OUTPUT_DIRECTORY "${game_project}/bin" LIBRARY_OUTPUT_DIRECTORY "${game_project}/bin")

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
set(splash_outputs)
foreach(name OpenGoldBoxSplashBackground OpenGoldBoxEngineLettering OpenGoldBoxGameLettering
    OpenGoldBoxEngineLettering.es OpenGoldBoxGameLettering.es)
    set(source "${PROJECT_SOURCE_DIR}/art/${name}.png")
    set(output "${game_project}/bin/splashes/${name}.png")
    add_custom_command(OUTPUT "${output}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${game_project}/bin/splashes"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${source}" "${output}"
        DEPENDS "${source}" VERBATIM)
    list(APPEND splash_outputs "${output}")
endforeach()
add_custom_target(opengoldbox_assets DEPENDS ${portrait_outputs} ${splash_outputs})

# Godot exports the executable; the C++ game code is its GDExtension.
# Rules remain loose files because the engine-independent loader uses filesystem I/O.
if(WIN32)
    set(package_commands
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data/rules/srd-5.2.1" "${game_output}/data/rules/srd-5.2.1"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${game_output}")
else()
    set(package_commands
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data/rules/srd-5.2.1" "${export_path}/Contents/MacOS/data/rules/srd-5.2.1"
        COMMAND codesign --force --deep --sign - "${export_path}"
        COMMAND "${CMAKE_COMMAND}" "-DAPP_BUNDLE=${export_path}" -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_macos_package.cmake")
endif()
add_custom_target(OpenGoldBox ALL
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/opengold.gdextension" "${game_project}/bin/opengold.gdextension"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${PROJECT_SOURCE_DIR}/data/rules/srd-5.2.1" "${game_project}/data/rules/srd-5.2.1"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${game_output}"
    COMMAND "${OPENGOLDBOX_GODOT}" --headless --editor --path "${game_project}" --import --quit
    COMMAND "${OPENGOLDBOX_GODOT}" --headless --path "${game_project}" --export-debug "${export_preset}" "${export_path}"
    ${package_commands}
    DEPENDS opengoldbox_game opengoldbox_assets
    VERBATIM)
