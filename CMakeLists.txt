cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(Tethys)

find_package(Vulkan REQUIRED)

# GLFW stuff
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Assimp stuff
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON)
set(ASSIMP_BUILD_TESTS OFF)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(external/assimp)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

if (UNIX)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

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
        "include/tethys/directional_light.hpp"
        "include/tethys/api/index_buffer.hpp"
        "include/tethys/model.hpp"
        "include/tethys/api/render_target.hpp")

set(TETHYS_SOURCES
        "src/tethys/api/instance.cpp"
        "src/tethys/util.cpp"
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
        "src/tethys/api/render_target.cpp" src/tethys/mesh.cpp)

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

target_compile_options(assimp INTERFACE -Wno-error)
if (UNIX)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_options(Tethys PUBLIC -O0 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-c99-extensions -Wno-shadow -Wno-deprecated-declarations -Wno-nullability-extension)
    else()
        target_compile_options(Tethys PUBLIC -O3 -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-c99-extensions -Wno-shadow -Wno-deprecated-declarations -Wno-nullability-extension)
    endif()
else()
    target_compile_options(Tethys PUBLIC /MP)
endif()

set(TETHYS_DEFINITIONS ${TETHYS_DEFINITIONS}
        VK_ENABLE_BETA_EXTENSIONS
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE)

if (WIN32)
    set(TETHYS_DEFINITIONS ${TETHYS_DEFINITIONS} _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS) # Goddamnit windows
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (UNIX)
        set(TETHYS_DEFINITIONS ${TETHYS_DEFINITIONS} _GLIBCXX_DEBUG)
    endif()

    set(TETHYS_DEFINITIONS ${TETHYS_DEFINITIONS} TETHYS_DEBUG)
endif()

target_compile_definitions(Tethys PUBLIC ${TETHYS_DEFINITIONS})

if (UNIX)
    target_link_libraries(Tethys PUBLIC glfw assimp dl)
else()
    target_link_libraries(Tethys PUBLIC glfw assimp)
endif()

target_precompile_headers(Tethys PUBLIC <vulkan/vulkan.hpp>)

file(GLOB SHADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag")
set(SHADER_OUTPUT_FILES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_FNAME ${SHADER} NAME)
    set(SHADER_OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_FNAME}.spv)
    list(APPEND SHADER_OUTPUT_FILES ${SHADER_OUTPUT})
    add_custom_command(OUTPUT ${SHADER_OUTPUT}
            COMMAND glslc --target-env=vulkan1.1 ${SHADER} "-o${SHADER_OUTPUT}"
            DEPENDS ${SHADER})
endforeach()

add_custom_target(TethysShaders
        ALL DEPENDS ${SHADER_OUTPUT_FILES}
        COMMENT "Building shaders")

add_dependencies(Tethys TethysShaders)

add_executable(TethysTestApp
        camera_util.hpp
        camera_util.cpp
        test_app.cpp)

target_include_directories(TethysTestApp PRIVATE include)
target_link_libraries(TethysTestApp PRIVATE Tethys)