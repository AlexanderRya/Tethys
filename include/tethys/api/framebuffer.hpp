#ifndef TETHYS_FRAMEBUFFER_HPP
#define TETHYS_FRAMEBUFFER_HPP

#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::api {
     std::vector<vk::Framebuffer> make_default_framebuffers(const vk::RenderPass);
} // namespace tethys::api

#endif //TETHYS_FRAMEBUFFER_HPP
