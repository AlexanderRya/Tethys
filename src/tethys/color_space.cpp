#include <tethys/color_space.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys {
    vk::Format to_vk_enum(const ColorSpace color_space) {
        switch (color_space) {
            case ColorSpace::eR8G8B8A8Unorm: {
                return vk::Format::eR8G8B8A8Unorm;
            }

            case ColorSpace::eR8G8B8A8Srgb: {
                return vk::Format::eR8G8B8A8Srgb;
            }
        }
    }
} // namespace tethys