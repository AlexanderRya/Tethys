cmake_minimum_required(VERSION 3.17)
project(Tethys)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

set(VMA_HEADERS
        "external/VulkanMemoryAllocator/src/vk_mem_alloc.h")

set(TETHYS_HEADERS
        ${VMA_HEADERS}
        "include/tethys/api/private/context.hpp"
        "include/tethys/forwards.hpp"
        "include/tethys/api/private/instance.hpp"
        "include/tethys/logger.hpp"
        "include/tethys/util.hpp"
        "include/tethys/types.hpp"
        "include/tethys/window/window.hpp"
        "include/tethys/api/private/device.hpp"
        "include/tethys/api/private/command_buffer.hpp"
        "include/tethys/api/private/command_pool.hpp"
        "include/tethys/api/private/descriptor_pool.hpp"
        "include/tethys/api/private/swapchain.hpp"
        "include/tethys/api/private/image.hpp"
        "include/tethys/api/private/render_pass.hpp"
        "include/tethys/api/private/framebuffer.hpp"
        "include/tethys/api/core.hpp"
        "include/tethys/renderer/renderer.hpp"
        "include/tethys/pipeline.hpp"
        "include/tethys/handle.hpp"
        "include/tethys/vertex.hpp"
        "include/tethys/mesh.hpp"
        "include/tethys/render_data.hpp"
        "include/tethys/api/private/static_buffer.hpp"
        "include/tethys/api/private/vertex_buffer.hpp"
        "include/tethys/constants.hpp"
        "include/tethys/api/private/single_buffer.hpp"
        "include/tethys/api/private/buffer.hpp"
        "include/tethys/api/private/single_descriptor_set.hpp"
        "include/tethys/api/private/descriptor_set.hpp"
        "include/tethys/texture.hpp"
        "include/tethys/api/private/sampler.hpp"
        "include/tethys/point_light.hpp"
        "include/tethys/camera.hpp"
        "include/tethys/material.hpp" include/tethys/directional_light.hpp include/tethys/acquire.hpp)

set(TETHYS_SOURCES
        "src/tethys/api/private/instance.cpp"
        "src/tethys/util.cpp"
        "src/tethys/api/private/context.cpp"
        "src/tethys/api/private/vma.cpp"
        "src/tethys/window/window.cpp"
        "src/tethys/api/private/device.cpp"
        "src/tethys/api/private/command_buffer.cpp"
        "src/tethys/api/private/command_pool.cpp"
        "src/tethys/api/private/descriptor_pool.cpp"
        "src/tethys/api/private/swapchain.cpp"
        "src/tethys/api/private/image.cpp"
        "src/tethys/api/private/render_pass.cpp"
        "src/tethys/api/private/framebuffer.cpp"
        "src/tethys/api/core.cpp"
        "src/tethys/renderer/renderer.cpp"
        "src/tethys/pipeline.cpp"
        "src/tethys/api/private/static_buffer.cpp"
        "src/tethys/api/private/vertex_buffer.cpp"
        "src/tethys/api/private/single_descriptor_set.cpp"
        "src/tethys/api/private/descriptor_set.cpp"
        "src/tethys/api/private/sampler.cpp"
        "src/tethys/texture.cpp"
        "src/tethys/api/private/stb_image.cpp")

add_library(Tethys STATIC
        ${TETHYS_HEADERS}
        ${TETHYS_SOURCES})

target_include_directories(Tethys PUBLIC
        include
        external/stb
        external/glm
        external/glfw/include
        ${Vulkan_INCLUDE_DIRS}
        external/VulkanMemoryAllocator/src)

if (UNIX)
    target_compile_options(Tethys PUBLIC -std=c++20 -O3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-c99-extensions -Wno-shadow -Wno-deprecated-declarations -Wno-nullability-extension)
endif()

if (WIN32)
    target_compile_definitions(Tethys PUBLIC _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if (UNIX)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_definitions(Tethys PUBLIC _GLIBCXX_DEBUG)
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Tethys PUBLIC TETHYS_DEBUG)
endif()

target_link_libraries(Tethys PUBLIC glfw)

file(GLOB TETHYS_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")
foreach(SHADER ${TETHYS_SHADERS})
    get_filename_component(SHADER_FNAME ${SHADER} NAME)
    add_custom_command(
            TARGET Tethys
            COMMAND echo Compiling shader ${SHADER_FNAME} &&
            glslc --target-env=vulkan1.1 ${SHADER} "-o${CMAKE_BINARY_DIR}/shaders/${SHADER_FNAME}.spv")
endforeach()
