#ifndef TETHYS_FRAMEBUFFER_HPP
#define TETHYS_FRAMEBUFFER_HPP

#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] std::vector<vk::Framebuffer> make_default_framebuffers();
} // namespace tethys::api

#endif //TETHYS_FRAMEBUFFER_HPP
