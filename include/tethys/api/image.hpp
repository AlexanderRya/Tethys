#ifndef TETHYS_IMAGE_HPP
#define TETHYS_IMAGE_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <filesystem>

namespace tethys::api {
    struct Image {
        struct CreateInfo {
            i32 width{};
            i32 height{};

            vk::Format format{};
            vk::ImageTiling tiling{};

            vk::ImageUsageFlags usage_flags{};
            VmaMemoryUsage memory_usage{};
        };

        vk::Image handle{};
        vk::Format format{};
        vk::ImageTiling tiling{};
        i32 width{};
        i32 height{};
        VmaAllocation allocation{};
    };

    [[nodiscard]] Image make_image(const Image::CreateInfo&);
    [[nodiscard]] vk::ImageView make_image_view(const vk::Image, const vk::Format, const vk::ImageAspectFlags);
    void transition_image_layout(vk::Image, const vk::ImageLayout, const vk::ImageLayout);
} // namespace tethys::api

#endif //TETHYS_IMAGE_HPP
