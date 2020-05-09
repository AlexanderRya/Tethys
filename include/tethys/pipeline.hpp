#ifndef TETHYS_PIPELINE_HPP
#define TETHYS_PIPELINE_HPP

#include <tethys/handle.hpp>

#include <vulkan/vulkan.hpp>

#include <filesystem>
#include <vector>

namespace tethys {
    namespace layout {
        template <u32 Idx>
        vk::DescriptorSetLayout& get();

        void load();
    } // namespace tethys::layout

    struct Pipeline {
        struct CreateInfo {
            std::vector<vk::DescriptorSetLayout> layouts{};
            vk::PushConstantRange push_constants{};

            std::string vertex{};
            std::string fragment{};

            u32 subpass_idx{};

            vk::RenderPass render_pass{};
            vk::SampleCountFlagBits samples{};
            vk::CullModeFlagBits cull{};
            std::vector<vk::DynamicState> dynamic_states{};
        };

        vk::Pipeline handle{};
        vk::PipelineLayout layout{};
    };

    [[nodiscard]] Pipeline make_pipeline(const Pipeline::CreateInfo&);
} // namespace tethys

#endif //TETHYS_PIPELINE_HPP
