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
            u32 mips{};

            vk::Format format{};
            vk::ImageTiling tiling{};
            vk::SampleCountFlagBits samples{};

            vk::ImageUsageFlags usage_flags{};
            VmaMemoryUsage memory_usage{};
        };

        i32 width{};
        i32 height{};
        vk::Image handle{};
        vk::Format format{};
        vk::ImageTiling tiling{};
        VmaAllocation allocation{};
        vk::SampleCountFlagBits samples{};
    };

    [[nodiscard]] Image make_image(const Image::CreateInfo&);
    [[nodiscard]] vk::ImageView make_image_view(const vk::Image, const vk::Format, const vk::ImageAspectFlags, const u32);
    void transition_image_layout(vk::Image, const vk::ImageLayout, const vk::ImageLayout, const u32);
} // namespace tethys::api

#endif //TETHYS_IMAGE_HPP
