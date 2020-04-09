#ifndef TETHYS_PIPELINE_HPP
#define TETHYS_PIPELINE_HPP

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    struct PipelineLayout {
        vk::PipelineLayout pipeline{};
        vk::DescriptorSetLayout set{};
    };

    struct Pipeline {
        PipelineLayout layout{};
        vk::Pipeline handle{};
    };

    [[nodiscard]] Pipeline make_generic_pipeline(const char*, const char*);
} // namespace tethys::api

#endif //TETHYS_PIPELINE_HPP
