#ifndef TETHYS_RENDER_PASS_HPP
#define TETHYS_RENDER_PASS_HPP

#include <tethys/forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::RenderPass make_offscreen_render_pass(const Offscreen&);
    [[nodiscard]] vk::RenderPass make_shadow_depth_render_pass(const ShadowDepth&);
} // namespace tethys::api

#endif //TETHYS_RENDER_PASS_HPP
