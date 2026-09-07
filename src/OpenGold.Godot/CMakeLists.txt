# Official bindings for the documented GDExtension boundary. Native-only builds
# do not need this dependency or its Python code-generation prerequisite.
include(FetchContent)
if(NOT Python3_EXECUTABLE AND EXISTS "${PROJECT_SOURCE_DIR}/build/_deps/python/python.exe")
    set(Python3_EXECUTABLE "${PROJECT_SOURCE_DIR}/build/_deps/python/python.exe")
endif()
find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE PATH "" FORCE)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${GODOTCPP_BUILD_PROFILE}")
set(GODOTCPP_DISABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
set(GODOTCPP_USE_HOT_RELOAD OFF CACHE BOOL "" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
FetchContent_Declare(godot_cpp
    GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
    GIT_TAG e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77
)
FetchContent_MakeAvailable(godot_cpp)
# Keep the approved source PNGs authoritative, and ship build copies inside
# the Godot project. Asset changes refresh even when the DLL needs no relink.
set(portrait_outputs)
foreach(species gnome orc goliath tiefling dragonborn)
    foreach(gender male female)
        set(portrait_source "${PROJECT_SOURCE_DIR}/data/art/portraits/${species}-${gender}.png")
        set(portrait_output "${PROJECT_SOURCE_DIR}/godot/bin/portraits/${species}-${gender}.png")
        add_custom_command(OUTPUT "${portrait_output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_SOURCE_DIR}/godot/bin/portraits"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${portrait_source}" "${portrait_output}"
            DEPENDS "${portrait_source}" VERBATIM)
        list(APPEND portrait_outputs "${portrait_output}")
    endforeach()
endforeach()
add_custom_target(opengold_portrait_assets DEPENDS ${portrait_outputs})
add_library(opengold_godot SHARED rolf_tour_view.cpp combat_view.cpp character_creation_view.cpp
    sound_board_view.cpp godot_sound_output.cpp register_types.cpp)
add_dependencies(opengold_godot opengold_portrait_assets)
target_link_libraries(opengold_godot PRIVATE opengold_core opengold_rules_srd5 godot-cpp)
target_compile_features(opengold_godot PRIVATE cxx_std_20)
target_compile_definitions(opengold_godot PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
set_target_properties(opengold_godot PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/godot/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/godot/bin"
)
# Register the extension only after its library exists, so native-only users
# can still open the existing Godot research tools without a missing-DLL error.
add_custom_command(TARGET opengold_godot POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/opengold.gdextension"
        "${PROJECT_SOURCE_DIR}/godot/bin/opengold.gdextension"
)
