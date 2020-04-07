cmake_minimum_required(VERSION 3.17)
project(Tethys CXX)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 20)

set(TETHYS_HEADERS
        "include/tethys/api/Context.hpp"
        "include/tethys/Forwards.hpp"
        "include/tethys/api/Instance.hpp"
        "include/tethys/Logger.hpp"
        "include/tethys/Util.hpp"
        "include/tethys/Types.hpp")

set(TETHYS_SOURCES
        "src/tethys/api/Instance.cpp"
        "src/tethys/Util.cpp"
        "src/tethys/api/Context.cpp")

add_library(Tethys
        ${TETHYS_HEADERS}
        ${TETHYS_SOURCES})

target_include_directories(Tethys PRIVATE
        include
        ${Vulkan_INCLUDE_DIRS})

if (WIN32)
    target_compile_definitions(Tethys PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if (UNIX)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_definitions(Tethys PRIVATE _GLIBCXX_DEBUG)
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Tethys PRIVATE TETHYS_DEBUG)
endif()