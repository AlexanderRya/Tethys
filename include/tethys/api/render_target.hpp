#ifndef TETHYS_RENDER_TARGET_HPP
#define TETHYS_RENDER_TARGET_HPP

#include <tethys/api/image.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    struct Offscreen {
        api::Image color{};
        vk::ImageView color_view{};

        api::Image depth{};
        vk::ImageView depth_view{};

        api::Image resolve{};
        vk::ImageView resolve_view{};
    };

    [[nodiscard]] Offscreen make_offscreen_target();
} // namespace tethys::api

#endif //TETHYS_RENDER_TARGET_HPP
