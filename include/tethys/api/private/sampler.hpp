#ifndef TETHYS_SAMPLER_HPP
#define TETHYS_SAMPLER_HPP

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    [[nodiscard]] vk::Sampler make_default_sampler();
} // namespace tethys::api

#endif //TETHYS_SAMPLER_HPP
