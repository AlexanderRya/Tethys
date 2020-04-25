#include <tethys/api/context.hpp>
#include <tethys/constants.hpp>
#include <tethys/pipeline.hpp>
#include <tethys/acquire.hpp>
#include <tethys/vertex.hpp>
#include <tethys/logger.hpp>

#include <fstream>
#include <vector>

namespace tethys {
    using namespace api;

    static std::vector<vk::DescriptorSetLayout> set_layouts;
    static std::vector<PipelineLayout> pipeline_layouts;

    void load_set_layouts() {
        set_layouts.resize(2);

        /* Minimal set layout */ {
            std::array<vk::DescriptorSetLayoutBinding, 3> layout_bindings{}; {
                layout_bindings[0].descriptorCount = 1;
                layout_bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
                layout_bindings[0].binding = binding::camera;
                layout_bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;

                layout_bindings[1].descriptorCount = 1;
                layout_bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
                layout_bindings[1].binding = binding::transform;
                layout_bindings[1].stageFlags = vk::ShaderStageFlagBits::eVertex;

                layout_bindings[2].descriptorCount = 1024;
                layout_bindings[2].descriptorType = vk::DescriptorType::eCombinedImageSampler;
                layout_bindings[2].binding = binding::texture;
                layout_bindings[2].stageFlags = vk::ShaderStageFlagBits::eFragment;
            }

            std::array<vk::DescriptorBindingFlags, 3> binding_flags{}; {
                binding_flags[0] = {};
                binding_flags[1] = {};
                binding_flags[2] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound;
            }

            vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{}; {
                binding_flags_info.bindingCount = binding_flags.size();
                binding_flags_info.pBindingFlags = binding_flags.data();
            }

            vk::DescriptorSetLayoutCreateInfo set_layout_create_info{}; {
                set_layout_create_info.pNext = &binding_flags_info;
                set_layout_create_info.bindingCount = layout_bindings.size();
                set_layout_create_info.pBindings = layout_bindings.data();
            }

            set_layouts[layout::minimal] = ctx.device.logical.createDescriptorSetLayout(set_layout_create_info, nullptr, ctx.dispatcher);
        }

        /* Generic set layout */ {
            std::array<vk::DescriptorSetLayoutBinding, 2> layout_bindings{}; {
                layout_bindings[0].descriptorCount = 1;
                layout_bindings[0].descriptorType = vk::DescriptorType::eStorageBuffer;
                layout_bindings[0].binding = binding::point_light;
                layout_bindings[0].stageFlags = vk::ShaderStageFlagBits::eFragment;

                layout_bindings[1].descriptorCount = 1;
                layout_bindings[1].descriptorType = vk::DescriptorType::eStorageBuffer;
                layout_bindings[1].binding = binding::directional_light;
                layout_bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
            }

            vk::DescriptorSetLayoutCreateInfo set_layout_create_info{}; {
                set_layout_create_info.bindingCount = layout_bindings.size();
                set_layout_create_info.pBindings = layout_bindings.data();
            }

            set_layouts[layout::generic] = ctx.device.logical.createDescriptorSetLayout(set_layout_create_info, nullptr, ctx.dispatcher);
        }
    }

    void load_pipeline_layouts() {
        pipeline_layouts.resize(2);

        /* Generic pipeline layout */ {
            vk::PushConstantRange range{}; {
                range.size = 6 * sizeof(u32);
                range.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
                range.offset = 0;
            }

            std::vector<vk::DescriptorSetLayout> sets(2); {
                sets[0] = set_layouts[layout::minimal];
                sets[1] = set_layouts[layout::generic];
            }

            vk::PipelineLayoutCreateInfo layout_create_info{}; {
                layout_create_info.setLayoutCount = sets.size();
                layout_create_info.pSetLayouts = sets.data();
                layout_create_info.pushConstantRangeCount = 1;
                layout_create_info.pPushConstantRanges = &range;
            }

            pipeline_layouts[layout::generic].pipeline = ctx.device.logical.createPipelineLayout(layout_create_info, nullptr, ctx.dispatcher);
            pipeline_layouts[layout::generic].sets = std::move(sets);
        }

        /* Minimal pipeline layout */ {
            vk::PushConstantRange range{}; {
                range.size = 2 * sizeof(u32);
                range.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
                range.offset = 0;
            }

            vk::PipelineLayoutCreateInfo layout_create_info{}; {
                layout_create_info.setLayoutCount = 1;
                layout_create_info.pSetLayouts = &set_layouts[layout::minimal];
                layout_create_info.pushConstantRangeCount = 1;
                layout_create_info.pPushConstantRanges = &range;
            }

            pipeline_layouts[layout::minimal].pipeline = ctx.device.logical.createPipelineLayout(layout_create_info, nullptr, ctx.dispatcher);
            pipeline_layouts[layout::minimal].sets = { set_layouts[layout::minimal] };
        }
    }

    [[nodiscard]] static vk::ShaderModule load_module(const std::filesystem::path& path) {
        std::ifstream in(path, std::fstream::binary);

        if (!in.is_open()) {
            throw std::runtime_error("Error, \"" + path.generic_string() + "\" file not found.");
        }

        std::string spv{ std::istreambuf_iterator<char>{ in }, {} };

        vk::ShaderModuleCreateInfo create_info{}; {
            create_info.codeSize = spv.size();
            create_info.pCode = reinterpret_cast<const u32*>(spv.data());
        }

        auto module = ctx.device.logical.createShaderModule(create_info, nullptr, ctx.dispatcher);

        logger::info("Module \"" + path.generic_string() + "\" successfully loaded");

        return module;
    }

    template <>
    PipelineLayout& acquire<PipelineLayout&>(const u32 idx) {
        return pipeline_layouts[idx];
    }

    template <>
    vk::DescriptorSetLayout& acquire<vk::DescriptorSetLayout>(const u32 idx) {
        return set_layouts[idx];
    }

    Pipeline make_pipeline(const Pipeline::CreateInfo& info) {
        std::array<vk::ShaderModule, 2> modules{}; {
            modules[0] = load_module(info.vertex);
            modules[1] = load_module(info.fragment);
        }

        std::array<vk::PipelineShaderStageCreateInfo, 2> stages{}; {
            /* Vertex Module */ {
                vk::PipelineShaderStageCreateInfo vertex_stage{}; {
                    vertex_stage.pName = "main";
                    vertex_stage.module = modules[0];
                    vertex_stage.stage = vk::ShaderStageFlagBits::eVertex;
                }

                stages[0] = vertex_stage;
            }

            /* Fragment Module */ {
                vk::PipelineShaderStageCreateInfo fragment_stage{}; {
                    fragment_stage.pName = "main";
                    fragment_stage.module = modules[1];
                    fragment_stage.stage = vk::ShaderStageFlagBits::eFragment;
                }

                stages[1] = fragment_stage;
            }
        }

        constexpr std::array dynamic_states{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

        vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{}; {
            dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
            dynamic_state_create_info.pDynamicStates = dynamic_states.data();
        }

        vk::VertexInputBindingDescription vertex_binding{}; {
            vertex_binding.binding = 0;
            vertex_binding.stride = sizeof(Vertex);
            vertex_binding.inputRate = vk::VertexInputRate::eVertex;
        }

        std::array<vk::VertexInputAttributeDescription, 5> vertex_attributes{}; {
            vertex_attributes[0].binding = 0;
            vertex_attributes[0].format = vk::Format::eR32G32B32Sfloat;
            vertex_attributes[0].location = 0;
            vertex_attributes[0].offset = offsetof(Vertex, pos);

            vertex_attributes[1].binding = 0;
            vertex_attributes[1].format = vk::Format::eR32G32B32Sfloat;
            vertex_attributes[1].location = 1;
            vertex_attributes[1].offset = offsetof(Vertex, norms);

            vertex_attributes[2].binding = 0;
            vertex_attributes[2].format = vk::Format::eR32G32Sfloat;
            vertex_attributes[2].location = 2;
            vertex_attributes[2].offset = offsetof(Vertex, uvs);

            vertex_attributes[3].binding = 0;
            vertex_attributes[3].format = vk::Format::eR32G32B32Sfloat;
            vertex_attributes[3].location = 3;
            vertex_attributes[3].offset = offsetof(Vertex, tangent);

            vertex_attributes[4].binding = 0;
            vertex_attributes[4].format = vk::Format::eR32G32B32Sfloat;
            vertex_attributes[4].location = 4;
            vertex_attributes[4].offset = offsetof(Vertex, bitangent);
        }

        vk::PipelineVertexInputStateCreateInfo vertex_input_info{}; {
            vertex_input_info.pVertexBindingDescriptions = &vertex_binding;
            vertex_input_info.vertexBindingDescriptionCount = 1;
            vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();
            vertex_input_info.vertexAttributeDescriptionCount = vertex_attributes.size();
        }

        vk::PipelineInputAssemblyStateCreateInfo input_assembly{}; {
            input_assembly.topology = vk::PrimitiveTopology::eTriangleList;
            input_assembly.primitiveRestartEnable = false;
        }

        vk::Viewport viewport{};
        vk::Rect2D scissor{};

        vk::PipelineViewportStateCreateInfo viewport_state{}; {
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = &viewport;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = &scissor;
        }

        vk::PipelineRasterizationStateCreateInfo rasterizer_state_info{}; {
            rasterizer_state_info.lineWidth = 1.0f;
            rasterizer_state_info.depthBiasEnable = false;
            rasterizer_state_info.depthClampEnable = false;
            rasterizer_state_info.rasterizerDiscardEnable = false;
            rasterizer_state_info.polygonMode = vk::PolygonMode::eFill;
            rasterizer_state_info.cullMode = vk::CullModeFlagBits::eNone;
            rasterizer_state_info.frontFace = vk::FrontFace::eCounterClockwise;
        }

        vk::PipelineMultisampleStateCreateInfo multisampling_state_info{}; {
            multisampling_state_info.alphaToCoverageEnable = false;
            multisampling_state_info.sampleShadingEnable = false;
            multisampling_state_info.alphaToOneEnable = false;
            multisampling_state_info.rasterizationSamples = ctx.device.max_samples;
            multisampling_state_info.minSampleShading = 0.2f;
            multisampling_state_info.pSampleMask = nullptr;
        }

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_info{}; {
            depth_stencil_info.stencilTestEnable = false;
            depth_stencil_info.depthTestEnable = true;
            depth_stencil_info.depthWriteEnable = true;
            depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
            depth_stencil_info.depthBoundsTestEnable = false;
            depth_stencil_info.minDepthBounds = 0.0f;
            depth_stencil_info.maxDepthBounds = 1.0f;
            depth_stencil_info.stencilTestEnable = false;
            depth_stencil_info.front = vk::StencilOpState{};
            depth_stencil_info.back = vk::StencilOpState{};
        }

        vk::PipelineColorBlendAttachmentState color_blend_attachment{}; {
            color_blend_attachment.blendEnable = true;
            color_blend_attachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA;
            color_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            color_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            color_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;
            color_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            color_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            color_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;
        }

        vk::PipelineColorBlendStateCreateInfo color_blend_info{}; {
            color_blend_info.attachmentCount = 1;
            color_blend_info.pAttachments = &color_blend_attachment;
            color_blend_info.logicOp = vk::LogicOp::eCopy;
            color_blend_info.logicOpEnable = false;
            color_blend_info.blendConstants[0] = 0.0f;
            color_blend_info.blendConstants[1] = 0.0f;
            color_blend_info.blendConstants[2] = 0.0f;
            color_blend_info.blendConstants[3] = 0.0f;
        }

        vk::GraphicsPipelineCreateInfo pipeline_info{}; {
            pipeline_info.stageCount = stages.size();
            pipeline_info.pStages = stages.data();
            pipeline_info.pVertexInputState = &vertex_input_info;
            pipeline_info.pInputAssemblyState = &input_assembly;
            pipeline_info.pViewportState = &viewport_state;
            pipeline_info.pRasterizationState = &rasterizer_state_info;
            pipeline_info.pMultisampleState = &multisampling_state_info;
            pipeline_info.pDepthStencilState = &depth_stencil_info;
            pipeline_info.pColorBlendState = &color_blend_info;
            pipeline_info.pDynamicState = &dynamic_state_create_info;
            pipeline_info.layout = pipeline_layouts[info.layout_idx].pipeline;
            pipeline_info.renderPass = info.render_pass;
            pipeline_info.subpass = info.subpass_idx;
            pipeline_info.basePipelineHandle = nullptr;
            pipeline_info.basePipelineIndex = -1;
        }

        Pipeline pipeline{};

        pipeline.handle = ctx.device.logical.createGraphicsPipeline(nullptr, pipeline_info, nullptr, ctx.dispatcher);
        pipeline.layout = pipeline_layouts[info.layout_idx];

        logger::info("Pipeline successfully created");

        for (const auto& module : modules) {
            ctx.device.logical.destroyShaderModule(module, nullptr, ctx.dispatcher);
        }

        return pipeline;
    }
} // namespace tethys