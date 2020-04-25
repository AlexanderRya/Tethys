#ifndef TETHYS_COLOR_SPACE_HPP
#define TETHYS_COLOR_SPACE_HPP

#include <tethys/forwards.hpp>

namespace tethys {
    enum class ColorSpace {
        eR8G8B8A8Unorm,
        eR8G8B8A8Srgb,
    };

    [[nodiscard]] vk::Format to_vk_enum(const ColorSpace);
} // namespace tethys

#endif //TETHYS_COLOR_SPACE_HPP
