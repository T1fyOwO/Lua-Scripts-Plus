cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(LuaScriptsPlus VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Set the GEODE_SDK environment variable.")
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

# Lua os.execute uses system(), which is unavailable on iOS.
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(LUA_LOSLIB "${lua_SOURCE_DIR}/src/loslib.c")

    if (NOT EXISTS "${LUA_LOSLIB}")
        message(FATAL_ERROR "Lua loslib.c was not found: ${LUA_LOSLIB}")
    endif()

    file(READ "${LUA_LOSLIB}" LUA_LOSLIB_CONTENT)

    set(LUA_SYSTEM_MACRO
        "#define l_system(cmd)   system(cmd)  /* default definition */"
    )

    string(FIND
        "${LUA_LOSLIB_CONTENT}"
        "${LUA_SYSTEM_MACRO}"
        LUA_SYSTEM_POSITION
    )

    if (LUA_SYSTEM_POSITION EQUAL -1)
        message(FATAL_ERROR "Could not find Lua's l_system macro in loslib.c")
    endif()

    string(REPLACE
        "${LUA_SYSTEM_MACRO}"
        "#define l_system(cmd)   (-1) /* os.execute unavailable on iOS */"
        LUA_SYSTEM_MACRO_PATCHED
        "${LUA_LOSLIB_CONTENT}"
    )

    file(WRITE "${LUA_LOSLIB}" "${LUA_SYSTEM_MACRO_PATCHED}")

    message(STATUS "Applied Lua iOS system() compatibility patch")
endif()

# Android32 compatibility
if (ANDROID AND ANDROID_ABI STREQUAL "armeabi-v7a")
    target_compile_options(lua_static PRIVATE -ULUA_USE_POSIX)
endif()

target_link_libraries(${PROJECT_NAME}
    geode-sdk
    lua_static
)

setup_geode_mod(${PROJECT_NAME})
