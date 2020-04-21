cmake_minimum_required(VERSION 3.16.5)
project(Tethys)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/assimp)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

set(VMA_HEADERS
        "external/VulkanMemoryAllocator/src/vk_mem_alloc.h")

set(TETHYS_HEADERS
        ${VMA_HEADERS}
        "include/tethys/api/context.hpp"
        "include/tethys/forwards.hpp"
        "include/tethys/api/instance.hpp"
        "include/tethys/logger.hpp"
        "include/tethys/util.hpp"
        "include/tethys/types.hpp"
        "include/tethys/window/window.hpp"
        "include/tethys/api/device.hpp"
        "include/tethys/api/command_buffer.hpp"
        "include/tethys/api/command_pool.hpp"
        "include/tethys/api/descriptor_pool.hpp"
        "include/tethys/api/swapchain.hpp"
        "include/tethys/api/image.hpp"
        "include/tethys/api/render_pass.hpp"
        "include/tethys/api/framebuffer.hpp"
        "include/tethys/api/core.hpp"
        "include/tethys/renderer/renderer.hpp"
        "include/tethys/pipeline.hpp"
        "include/tethys/handle.hpp"
        "include/tethys/vertex.hpp"
        "include/tethys/mesh.hpp"
        "include/tethys/render_data.hpp"
        "include/tethys/api/static_buffer.hpp"
        "include/tethys/api/vertex_buffer.hpp"
        "include/tethys/constants.hpp"
        "include/tethys/api/single_buffer.hpp"
        "include/tethys/api/buffer.hpp"
        "include/tethys/api/single_descriptor_set.hpp"
        "include/tethys/api/descriptor_set.hpp"
        "include/tethys/texture.hpp"
        "include/tethys/api/sampler.hpp"
        "include/tethys/point_light.hpp"
        "include/tethys/camera.hpp"
        "include/tethys/material.hpp"
        "include/tethys/directional_light.hpp"
        "include/tethys/acquire.hpp"
        "include/tethys/model.hpp"
        "include/tethys/api/index_buffer.hpp"
        "include/tethys/experimental/rtx_renderer.hpp")

set(TETHYS_SOURCES
        "src/tethys/api/instance.cpp"
        "src/tethys/util.cpp"
        "src/tethys/api/context.cpp"
        "src/tethys/api/vma.cpp"
        "src/tethys/window/window.cpp"
        "src/tethys/api/device.cpp"
        "src/tethys/api/command_buffer.cpp"
        "src/tethys/api/command_pool.cpp"
        "src/tethys/api/descriptor_pool.cpp"
        "src/tethys/api/swapchain.cpp"
        "src/tethys/api/image.cpp"
        "src/tethys/api/render_pass.cpp"
        "src/tethys/api/framebuffer.cpp"
        "src/tethys/api/core.cpp"
        "src/tethys/renderer/renderer.cpp"
        "src/tethys/pipeline.cpp"
        "src/tethys/api/static_buffer.cpp"
        "src/tethys/api/vertex_buffer.cpp"
        "src/tethys/api/single_descriptor_set.cpp"
        "src/tethys/api/descriptor_set.cpp"
        "src/tethys/api/sampler.cpp"
        "src/tethys/texture.cpp"
        "src/tethys/api/stb_image.cpp"
        "src/tethys/api/index_buffer.cpp"
        "src/tethys/model.cpp"
        "src/tethys/experimental/rtx_renderer.cpp")

add_library(Tethys STATIC
        ${TETHYS_HEADERS}
        ${TETHYS_SOURCES})

target_include_directories(Tethys PUBLIC
        include
        external/stb
        external/glm
        external/glfw/include
        ${Vulkan_INCLUDE_DIRS}
        external/assimp/include
        external/VulkanMemoryAllocator/src)

if (UNIX)
    target_compile_options(Tethys PUBLIC -std=c++20 -O3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-c99-extensions -Wno-shadow -Wno-deprecated-declarations -Wno-nullability-extension)
endif()

set(TETHYS_COMPILE_DEFINITIONS ${TETHYS_COMPILE_DEFINITIONS} VK_ENABLE_BETA_EXTENSIONS)

if (WIN32)
    set(TETHYS_COMPILE_DEFINITIONS ${TETHYS_COMPILE_DEFINITIONS} _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

if (UNIX)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        set(TETHYS_COMPILE_DEFINITIONS ${TETHYS_COMPILE_DEFINITIONS} _GLIBCXX_DEBUG)
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(TETHYS_COMPILE_DEFINITIONS ${TETHYS_COMPILE_DEFINITIONS} TETHYS_DEBUG)
endif()

target_compile_definitions(Tethys PRIVATE ${TETHYS_COMPILE_DEFINITIONS})
target_link_libraries(Tethys PUBLIC glfw assimp)

file(GLOB TETHYS_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")
foreach(SHADER ${TETHYS_SHADERS})
    get_filename_component(SHADER_FILENAME ${SHADER} NAME)
    add_custom_command(
            TARGET Tethys
            COMMAND echo Compiling shader ${SHADER_FILENAME} &&
            glslc --target-env=vulkan1.1 ${SHADER} "-o${CMAKE_BINARY_DIR}/shaders/${SHADER_FILENAME}.spv")
endforeach()