#include <tethys/api/context.hpp>
#include <tethys/constants.hpp>
#include <tethys/pipeline.hpp>
#include <tethys/vertex.hpp>
#include <tethys/logger.hpp>

#include <fstream>
#include <vector>

namespace tethys {
    using namespace api;

    namespace layout {
        static std::vector<vk::DescriptorSetLayout> set_layouts;

        template <>
        vk::DescriptorSetLayout& get<layout::generic>() {
            return set_layouts[layout::generic];
        }

        template <>
        vk::DescriptorSetLayout& get<layout::minimal>() {
            return set_layouts[layout::minimal];
        }

        void load() {
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

                layout::get<layout::minimal>() = context.device.logical.createDescriptorSetLayout(set_layout_create_info, nullptr, context.dispatcher);
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

                layout::get<layout::generic>() = context.device.logical.createDescriptorSetLayout(set_layout_create_info, nullptr, context.dispatcher);
            }
        }
    } // namespace tethys::layout


    [[nodiscard]] static vk::ShaderModule load_module(const std::string& path) {
        std::ifstream in(path, std::fstream::binary);

        if (!in.is_open()) {
            throw std::runtime_error("Error, \"" + path + "\" file not found.");
        }

        std::string spv{ std::istreambuf_iterator<char>{ in }, {} };

        vk::ShaderModuleCreateInfo create_info{}; {
            create_info.codeSize = spv.size();
            create_info.pCode = reinterpret_cast<const u32*>(spv.data());
        }

        auto module = context.device.logical.createShaderModule(create_info, nullptr, context.dispatcher);

        logger::info("Module \"" + path + "\" successfully loaded");

        return module;
    }

    Pipeline make_pipeline(const Pipeline::CreateInfo& info) {
        Pipeline pipeline{};

        vk::PipelineLayoutCreateInfo layout_create_info{}; {
            if (info.push_constants.size != 0) {
                layout_create_info.pushConstantRangeCount = 1;
                layout_create_info.pPushConstantRanges = &info.push_constants;
            }
            if (!info.layouts.empty()) {
                layout_create_info.setLayoutCount = info.layouts.size();
                layout_create_info.pSetLayouts = info.layouts.data();
            }
        }
        pipeline.layout = context.device.logical.createPipelineLayout(layout_create_info, nullptr, context.dispatcher);

        std::array<vk::ShaderModule, 2> modules{}; {
            modules[0] = load_module(info.vertex);
            modules[1] = load_module(info.fragment);
        }

        std::array<vk::PipelineShaderStageCreateInfo, 2> stages{}; {
            stages[0].pName = "main";
            stages[0].module = modules[0];
            stages[0].stage = vk::ShaderStageFlagBits::eVertex;

            stages[1].pName = "main";
            stages[1].module = modules[1];
            stages[1].stage = vk::ShaderStageFlagBits::eFragment;
        }

        vk::PipelineDynamicStateCreateInfo dynamic_state_create_info{}; {
            dynamic_state_create_info.dynamicStateCount = info.dynamic_states.size();
            dynamic_state_create_info.pDynamicStates = info.dynamic_states.data();
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
            rasterizer_state_info.depthBiasEnable = true;
            rasterizer_state_info.depthBiasSlopeFactor = 0.0f;
            rasterizer_state_info.depthBiasConstantFactor = 0.0f;
            rasterizer_state_info.depthClampEnable = false;
            rasterizer_state_info.rasterizerDiscardEnable = false;
            rasterizer_state_info.polygonMode = vk::PolygonMode::eFill;
            rasterizer_state_info.cullMode = info.cull;
            rasterizer_state_info.frontFace = vk::FrontFace::eClockwise;
        }

        if (info.samples > context.device.samples) {
            throw std::runtime_error("Invalid number of samples requested in pipeline creation.");
        }

        vk::PipelineMultisampleStateCreateInfo multisampling_state_info{}; {
            multisampling_state_info.alphaToCoverageEnable = false;
            multisampling_state_info.sampleShadingEnable = true;
            multisampling_state_info.alphaToOneEnable = false;
            multisampling_state_info.rasterizationSamples = info.samples;
            multisampling_state_info.minSampleShading = 0.2f;
            multisampling_state_info.pSampleMask = nullptr;
        }

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_info{}; {
            depth_stencil_info.stencilTestEnable = false;
            depth_stencil_info.depthTestEnable = true;
            depth_stencil_info.depthWriteEnable = true;
            depth_stencil_info.depthCompareOp = vk::CompareOp::eLessOrEqual;
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
            pipeline_info.layout = pipeline.layout;
            pipeline_info.renderPass = info.render_pass;
            pipeline_info.subpass = info.subpass_idx;
            pipeline_info.basePipelineHandle = nullptr;
            pipeline_info.basePipelineIndex = -1;
        }

        pipeline.handle = context.device.logical.createGraphicsPipeline(nullptr, pipeline_info, nullptr, context.dispatcher);

        logger::info("Pipeline successfully created");

        for (const auto& module : modules) {
            context.device.logical.destroyShaderModule(module, nullptr, context.dispatcher);
        }

        return pipeline;
    }
} // namespace tethys