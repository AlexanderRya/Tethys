cmake_minimum_required(VERSION 3.17)
project(Tethys CXX)

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
        "include/tethys/api/private/Context.hpp"
        "include/tethys/Forwards.hpp"
        "include/tethys/api/private/Instance.hpp"
        "include/tethys/Logger.hpp"
        "include/tethys/Util.hpp"
        "include/tethys/Types.hpp"
        "include/tethys/window/Window.hpp"
        "include/tethys/api/private/Device.hpp"
        "include/tethys/api/private/CommandBuffer.hpp"
        "include/tethys/api/private/CommandPool.hpp"
        "include/tethys/api/private/DescriptorPool.hpp"
        "include/tethys/api/private/Swapchain.hpp"
        "include/tethys/api/private/Image.hpp"
        "include/tethys/api/private/RenderPass.hpp"
        "include/tethys/api/private/Framebuffer.hpp"
        "include/tethys/api/Core.hpp"
        "include/tethys/renderer/Renderer.hpp"
        "include/tethys/api/private/Pipeline.hpp"
        "include/tethys/Handle.hpp"
        "include/tethys/Vertex.hpp"
        "include/tethys/Mesh.hpp"
        "include/tethys/renderer/DrawCommand.hpp"
        "include/tethys/api/private/Buffer.hpp"
        "include/tethys/api/private/VertexBuffer.hpp")

set(TETHYS_SOURCES
        "src/tethys/api/private/Instance.cpp"
        "src/tethys/Util.cpp"
        "src/tethys/api/private/Context.cpp"
        "src/tethys/api/private/VMA.cpp"
        "src/tethys/window/Window.cpp"
        "src/tethys/api/private/Device.cpp"
        "src/tethys/api/private/CommandBuffer.cpp"
        "src/tethys/api/private/CommandPool.cpp"
        "src/tethys/api/private/DescriptorPool.cpp"
        "src/tethys/api/private/Swapchain.cpp"
        "src/tethys/api/private/Image.cpp"
        "src/tethys/api/private/RenderPass.cpp"
        "src/tethys/api/private/Framebuffer.cpp"
        "src/tethys/api/Core.cpp"
        "src/tethys/renderer/Renderer.cpp"
        "src/tethys/api/private/Pipeline.cpp"
        "src/tethys/api/private/Buffer.cpp"
        "src/tethys/api/private/VertexBuffer.cpp")

add_library(Tethys STATIC
        ${TETHYS_HEADERS}
        ${TETHYS_SOURCES})

target_include_directories(Tethys PRIVATE
        include
        external/glm
        external/glfw/include
        ${Vulkan_INCLUDE_DIRS}
        external/VulkanMemoryAllocator/src)

if (UNIX)
    target_compile_options(Tethys PRIVATE -std=c++20 -O3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-c99-extensions -Wno-shadow -Wno-deprecated-declarations -Wno-nullability-extension)
endif()

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

target_link_libraries(Tethys PRIVATE glfw)