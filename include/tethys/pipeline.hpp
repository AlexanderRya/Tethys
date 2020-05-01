#ifndef TETHYS_PIPELINE_HPP
#define TETHYS_PIPELINE_HPP

#include <tethys/handle.hpp>

#include <vulkan/vulkan.hpp>

#include <filesystem>
#include <vector>

namespace tethys {
    struct PipelineLayout {
        vk::PipelineLayout pipeline{};
        std::vector<vk::DescriptorSetLayout> sets{};
    };

    struct Pipeline {
        struct CreateInfo {
            std::string vertex{};
            std::string fragment{};

            u32 layout_idx{};
            u32 subpass_idx{};

            vk::RenderPass render_pass{};
            vk::SampleCountFlagBits samples{};
            vk::CullModeFlagBits cull{};
            std::vector<vk::DynamicState> dynamic_states{};
        };

        vk::Pipeline handle{};
        PipelineLayout layout{};
    };

    void load_set_layouts();
    void load_pipeline_layouts();
    [[nodiscard]] Pipeline make_pipeline(const Pipeline::CreateInfo&);
} // namespace tethys

#endif //TETHYS_PIPELINE_HPP
