#ifndef TETHYS_FRAMEBUFFER_HPP
#define TETHYS_FRAMEBUFFER_HPP

#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] vk::Framebuffer make_offscreen_framebuffer(const Offscreen&, const vk::RenderPass);
    [[nodiscard]] vk::Framebuffer make_shadow_depth_framebuffer(const ShadowDepth&, const vk::RenderPass);
} // namespace tethys::api

#endif //TETHYS_FRAMEBUFFER_HPP
