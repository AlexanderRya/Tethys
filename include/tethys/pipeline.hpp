#ifndef TETHYS_PIPELINE_HPP
#define TETHYS_PIPELINE_HPP

#include <tethys/handle.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys {
    struct PipelineLayout {
        vk::PipelineLayout pipeline{};
        std::vector<vk::DescriptorSetLayout> sets{};
    };

    struct Pipeline {
        vk::Pipeline handle{};
        PipelineLayout layout{};
    };

    void load_all_builtin_shaders();
} // namespace tethys

#endif //TETHYS_PIPELINE_HPP
