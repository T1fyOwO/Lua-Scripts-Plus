cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(LuaScriptsPlus VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/main.cpp src/LuaEngine.cpp)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Set the GEODE_SDK environment variable.")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Lua 5.4 via CPM (bundled with the Geode SDK)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
CPMAddPackage(
    NAME lua
    GITHUB_REPOSITORY walterschell/Lua
    GIT_TAG v5.4.5
    OPTIONS "LUA_ENABLE_SHARED OFF" "LUA_BUILD_COMPILER OFF" "LUA_BUILD_INTERPRETER OFF"
)
target_link_libraries(${PROJECT_NAME} lua_static)
