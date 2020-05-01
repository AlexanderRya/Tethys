#ifndef TETHYS_SAMPLER_HPP
#define TETHYS_SAMPLER_HPP

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    enum class SamplerType {
        eDefault,
        eDepth
    };

    void make_samplers();
    [[nodiscard]] vk::Sampler sampler_from_type(const SamplerType&);
} // namespace tethys::api

#endif //TETHYS_SAMPLER_HPP
