#ifndef TETHYS_PIPELINE_HPP
#define TETHYS_PIPELINE_HPP

#include <tethys/handle.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys {
    struct PipelineLayout {
        vk::PipelineLayout pipeline{};
        vk::DescriptorSetLayout set{};
    };

    struct Pipeline {
        vk::Pipeline handle{};
        u32 layout_idx{};
    };

    void load_all_builtin_shaders();
    [[nodiscard]] Pipeline& get_shader(const u32);
    [[nodiscard]] PipelineLayout& get_layout(const u32);
} // namespace tethys

#endif //TETHYS_PIPELINE_HPP
