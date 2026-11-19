cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(LuaScriptsPlus VERSION 1.0.0)

# Stops CMake from scanning for C++ modules (fixes the Android64 fmt/std.cc error)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Set GEODE_SDK.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    src/LuaEngine.cpp
)

# Lua 5.4 via CPM
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

CPMAddPackage(
    NAME lua
    GITHUB_REPOSITORY walterschell/Lua
    GIT_TAG v5.4.5
    OPTIONS
        "LUA_ENABLE_SHARED OFF"
        "LUA_BUILD_COMPILER OFF"
        "LUA_BUILD_INTERPRETER OFF"
)

# iOS compatibility (Lua disables os.execute itself when LUA_USE_IOS is set)
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(lua_static PRIVATE LUA_USE_IOS)
endif()

# Android32 compatibility
if (ANDROID AND ANDROID_ABI STREQUAL "armeabi-v7a")
    target_compile_options(lua_static PRIVATE -ULUA_USE_POSIX)
endif()

# Dear ImGui for the script runner window
CPMAddPackage("gh:matcool/gd-imgui-cocos#934958c68b1f050a8976d893584fa71368c1df1f")

# gd-imgui-cocos includes the CCKeyboardDispatcher modify header on every platform,
# but that header doesn't compile on iOS (no ctor/dtor address). iOS never uses it.
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(IMGUI_COCOS_HOOKS "${gd-imgui-cocos_SOURCE_DIR}/src/hooks.cpp")
    if (EXISTS "${IMGUI_COCOS_HOOKS}")
        file(READ "${IMGUI_COCOS_HOOKS}" IMGUI_COCOS_HOOKS_CONTENT)
        string(REPLACE
            "#include <Geode/modify/CCKeyboardDispatcher.hpp>"
            "#ifndef GEODE_IS_IOS\n#include <Geode/modify/CCKeyboardDispatcher.hpp>\n#endif"
            IMGUI_COCOS_HOOKS_PATCHED
            "${IMGUI_COCOS_HOOKS_CONTENT}")
        file(WRITE "${IMGUI_COCOS_HOOKS}" "${IMGUI_COCOS_HOOKS_PATCHED}")
        message(STATUS "Patched gd-imgui-cocos hooks.cpp for iOS")
    else()
        message(WARNING "gd-imgui-cocos hooks.cpp not found at ${IMGUI_COCOS_HOOKS}")
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
    geode-sdk
    lua_static
    imgui-cocos
)

setup_geode_mod(${PROJECT_NAME})
