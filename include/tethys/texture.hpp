#ifndef TETHYS_TEXTURE_HPP
#define TETHYS_TEXTURE_HPP

#include <tethys/api/image.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys {
    struct Texture {
        api::Image image{};
        u32 index{}; // Temporary fix, indicates texture index in texture descriptor
        u32 mips{};

        [[nodiscard]] vk::DescriptorImageInfo info(const api::SamplerType&) const;
    };

    [[nodiscard]] Texture load_texture(const char*, const vk::Format);
    [[nodiscard]] Texture load_texture(const u8*, const u32, const u32, const u32, const vk::Format);
} // namespace tethys

#endif //TETHYS_TEXTURE_HPP