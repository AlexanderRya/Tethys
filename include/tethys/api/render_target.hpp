#ifndef TETHYS_RENDER_TARGET_HPP
#define TETHYS_RENDER_TARGET_HPP

#include <tethys/api/image.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    struct Offscreen {
        api::Image color{};
        api::Image depth{};
        api::Image msaa{};
    };

    [[nodiscard]] Offscreen make_offscreen_target();
} // namespace tethys::api

#endif //TETHYS_RENDER_TARGET_HPP
