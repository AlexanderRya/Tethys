#ifndef TETHYS_RENDER_TARGET_HPP
#define TETHYS_RENDER_TARGET_HPP

#include <tethys/api/image.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    struct Offscreen {
        api::Image image{};
        vk::ImageView image_view{};

        api::Image depth_image{};
        vk::ImageView depth_view{};

        api::Image msaa_image{};
        vk::ImageView msaa_view{};
    };

    struct ShadowDepth {
        api::Image image{};
        vk::ImageView view{};
    };

    [[nodiscard]] Offscreen make_offscreen_target();
    [[nodiscard]] ShadowDepth make_shadow_depth_target();
} // namespace tethys::api

#endif //TETHYS_RENDER_TARGET_HPP
