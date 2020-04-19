#ifndef TETHYS_TEXTURE_HPP
#define TETHYS_TEXTURE_HPP

#include <tethys/api/private/image.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys {
    class Texture {
        api::Image image{};
        vk::ImageView view{};
    public:
        Texture() = default;

        void load(const char*);

        [[nodiscard]] vk::DescriptorImageInfo info() const;
    };
} // namespace tethys

#endif //TETHYS_TEXTURE_HPP
