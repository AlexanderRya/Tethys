#ifndef TETHYS_TEXTURE_HPP
#define TETHYS_TEXTURE_HPP

#include <tethys/api/image.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys {
    struct Texture {
        api::Image image{};
        vk::ImageView view{};

        [[nodiscard]] vk::DescriptorImageInfo info() const;
    };

    [[nodiscard]] Texture load_texture(const char*);
    [[nodiscard]] Texture load_texture(const u8*, const u32, const u32, const u32);
} // namespace tethys

#endif //TETHYS_TEXTURE_HPP