#ifndef TETHYS_RENDERPASS_HPP
#define TETHYS_RENDERPASS_HPP

#include <tethys/Forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::RenderPass make_default_render_pass();
} // namespace tethys::api

#endif //TETHYS_RENDERPASS_HPP
