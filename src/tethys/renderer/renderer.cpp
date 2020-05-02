#include <tethys/api/command_buffer.hpp>
#include <tethys/api/descriptor_set.hpp>
#include <tethys/renderer/renderer.hpp>
#include <tethys/directional_light.hpp>
#include <tethys/api/vertex_buffer.hpp>
#include <tethys/api/static_buffer.hpp>
#include <tethys/api/render_target.hpp>
#include <tethys/api/index_buffer.hpp>
#include <tethys/api/render_pass.hpp>
#include <tethys/api/framebuffer.hpp>
#include <tethys/api/sampler.hpp>
#include <tethys/point_light.hpp>
#include <tethys/render_data.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/buffer.hpp>
#include <tethys/constants.hpp>
#include <tethys/pipeline.hpp>
#include <tethys/texture.hpp>
#include <tethys/acquire.hpp>
#include <tethys/model.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <stack>

namespace tethys::renderer {
    using namespace api;

    static vk::RenderPass offscreen_render_pass{};
    static vk::Framebuffer offscreen_framebuffer{};

    static vk::RenderPass shadow_depth_render_pass{};
    static vk::Framebuffer shadow_depth_framebuffer{};

    static std::vector<vk::Semaphore> image_available{};
    static std::vector<vk::Semaphore> render_finished{};
    static std::vector<vk::Fence> in_flight{};

    static std::vector<vk::CommandBuffer> command_buffers{};

    static u32 image_index{};
    static u32 current_frame{};

    static std::stack<usize> free_vertex_buffers{};
    static std::vector<api::VertexBuffer> vertex_buffers{};

    static std::stack<usize> free_index_buffers{};
    static std::vector<api::IndexBuffer> index_buffers{};

    static Offscreen offscreen{};
    static ShadowDepth shadow_depth{};

    static std::vector<Texture> textures{};
    static std::vector<Model> models{};

    // Part of set 0
    static api::Buffer<Camera> camera_buffer{};
    static api::Buffer<glm::mat4> transform_buffer{};
    // Part of set 1
    static api::Buffer<PointLight> point_light_buffer{};
    static api::Buffer<DirectionalLight> directional_light_buffer{};
    static api::Buffer<glm::mat4> light_space_buffer{};

    static api::DescriptorSet generic_set{};
    static api::DescriptorSet minimal_set{};
    static api::DescriptorSet shadow_set{};

    static Pipeline generic;
    static Pipeline minimal;
    static Pipeline shadow;
    static Pipeline debug;

    static Handle<Mesh> debug_quad;

    static void update_textures() {
        std::vector<vk::DescriptorImageInfo> image_info{};
        image_info.reserve(textures.size());

        for (const auto& texture : textures) {
            image_info.emplace_back(texture.info(SamplerType::eDefault));
        }

        api::UpdateImageInfo info{}; {
            info.image = std::move(image_info);
            info.binding = binding::texture;
            info.type = vk::DescriptorType::eCombinedImageSampler;
        }

        minimal_set.update(info);
    }

    void initialise() {
        offscreen = api::make_offscreen_target();
        offscreen_render_pass = api::make_offscreen_render_pass(offscreen);
        offscreen_framebuffer = api::make_offscreen_framebuffer(offscreen, offscreen_render_pass);

        shadow_depth = api::make_shadow_depth_target();
        shadow_depth_render_pass = api::make_shadow_depth_render_pass(shadow_depth);
        shadow_depth_framebuffer = api::make_shadow_depth_framebuffer(shadow_depth, shadow_depth_render_pass);

        command_buffers = api::make_rendering_command_buffers();

        vk::SemaphoreCreateInfo semaphore_create_info{};

        image_available.reserve(frames_in_flight);
        render_finished.reserve(frames_in_flight);

        for (u64 i = 0; i < frames_in_flight; ++i) {
            image_available.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
            render_finished.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
        }

        in_flight.resize(frames_in_flight, nullptr);

        load_set_layouts();
        load_pipeline_layouts();

        Pipeline::CreateInfo generic_info{}; {
            generic_info.vertex = "shaders/generic.vert.spv";
            generic_info.fragment = "shaders/generic.frag.spv";
            generic_info.subpass_idx = 0;
            generic_info.layout_idx = layout::generic;
            generic_info.render_pass = offscreen_render_pass;
            generic_info.samples = ctx.device.max_samples;
            generic_info.cull = vk::CullModeFlagBits::eNone;
            generic_info.dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };
        }
        generic = make_pipeline(generic_info);

        Pipeline::CreateInfo minimal_info{}; {
            minimal_info.vertex = "shaders/minimal.vert.spv";
            minimal_info.fragment = "shaders/minimal.frag.spv";
            minimal_info.subpass_idx = 0;
            minimal_info.render_pass = offscreen_render_pass;
            minimal_info.layout_idx = layout::minimal;
            minimal_info.samples = ctx.device.max_samples;
            generic_info.cull = vk::CullModeFlagBits::eNone;
            minimal_info.dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };
        }
        minimal = make_pipeline(minimal_info);

        Pipeline::CreateInfo shadow_info{}; {
            shadow_info.vertex = "shaders/shadow.vert.spv";
            shadow_info.fragment = "shaders/shadow.frag.spv";
            shadow_info.subpass_idx = 0;
            shadow_info.render_pass = shadow_depth_render_pass;
            shadow_info.layout_idx = layout::shadow;
            shadow_info.samples = vk::SampleCountFlagBits::e1;
            shadow_info.cull = vk::CullModeFlagBits::eFront;
            shadow_info.dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor,
                vk::DynamicState::eDepthBias
            };
        }
        shadow = make_pipeline(shadow_info);

        Pipeline::CreateInfo debug_info{}; {
            debug_info.vertex = "shaders/debug.vert.spv";
            debug_info.fragment = "shaders/debug.frag.spv";
            debug_info.subpass_idx = 0;
            debug_info.render_pass = offscreen_render_pass;
            debug_info.layout_idx = layout::generic;
            debug_info.samples = ctx.device.max_samples;
            debug_info.cull = vk::CullModeFlagBits::eNone;
            debug_info.dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor
            };
        }
        debug = make_pipeline(debug_info);

        debug_quad = write_geometry(generate_debug_quad());

        camera_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);
        transform_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        point_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        directional_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        light_space_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);

        generic_set.create(acquire<vk::DescriptorSetLayout>(layout::generic));
        minimal_set.create(acquire<vk::DescriptorSetLayout>(layout::minimal));
        shadow_set.create(acquire<vk::DescriptorSetLayout>(layout::shadow));

        std::vector<api::UpdateBufferInfo> minimal_update(2); {
            minimal_update[0].binding = binding::camera;
            minimal_update[0].type = vk::DescriptorType::eUniformBuffer;
            minimal_update[0].buffers = camera_buffer.info();

            minimal_update[1].binding = binding::transform;
            minimal_update[1].type = vk::DescriptorType::eStorageBuffer;
            minimal_update[1].buffers = transform_buffer.info();
        }

        minimal_set.update(minimal_update);

        std::vector<api::UpdateBufferInfo> shadow_update(2); {
            shadow_update[0].binding = binding::camera;
            shadow_update[0].type = vk::DescriptorType::eUniformBuffer;
            shadow_update[0].buffers = light_space_buffer.info();

            shadow_update[1].binding = binding::transform;
            shadow_update[1].type = vk::DescriptorType::eStorageBuffer;
            shadow_update[1].buffers = transform_buffer.info();
        }

        shadow_set.update(shadow_update);

        std::vector<api::UpdateBufferInfo> generic_update(3); {
            generic_update[0].binding = binding::point_light;
            generic_update[0].type = vk::DescriptorType::eStorageBuffer;
            generic_update[0].buffers = point_light_buffer.info();

            generic_update[1].binding = binding::directional_light;
            generic_update[1].type = vk::DescriptorType::eStorageBuffer;
            generic_update[1].buffers = directional_light_buffer.info();

            generic_update[2].binding = binding::light_space;
            generic_update[2].type = vk::DescriptorType::eUniformBuffer;
            generic_update[2].buffers = light_space_buffer.info();
        }

        generic_set.update(generic_update);

        std::array<u8, 4> white = { 255, 255, 255, 255 };
        textures.emplace_back(load_texture(white.data(), 1, 1, 4, vk::Format::eR8G8B8A8Srgb));

        std::array<u8, 4> black{};
        textures.emplace_back(load_texture(black.data(), 1, 1, 4, vk::Format::eR8G8B8A8Srgb));

        update_textures();

        api::SingleUpdateImageInfo shadow_map_update{}; {
            shadow_map_update.image.imageView = shadow_depth.view;
            shadow_map_update.image.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            shadow_map_update.image.sampler = sampler_from_type(SamplerType::eDepth);
            shadow_map_update.binding = binding::shadow_map;
            shadow_map_update.type = vk::DescriptorType::eCombinedImageSampler;
        }

        generic_set.update(shadow_map_update);
    }

    Handle<Mesh> write_geometry(const Mesh& mesh) {
        return write_geometry(mesh.vertices, mesh.indices);
    }

    Handle<Mesh> write_geometry(const std::vector<Vertex>& geometry, const std::vector<u32>& indices) {
        usize vbo_index, ibo_index;

        if (!free_vertex_buffers.empty()) {
            vbo_index = free_vertex_buffers.top();
            vertex_buffers[vbo_index] = api::make_vertex_buffer(geometry);
            free_vertex_buffers.pop();
        } else {
            vertex_buffers.emplace_back(api::make_vertex_buffer(geometry));
            vbo_index = vertex_buffers.size() - 1;
        }

        if (!free_index_buffers.empty()) {
            ibo_index = free_index_buffers.top();
            index_buffers[ibo_index] = api::make_index_buffer(indices);
            free_index_buffers.pop();
        } else {
            index_buffers.emplace_back(api::make_index_buffer(indices));
            ibo_index = index_buffers.size() - 1;
        }

        return Handle<Mesh>{ vbo_index, ibo_index };
    }

    Handle<Texture> upload_texture(const char* path, const vk::Format color_space) {
        textures.emplace_back(load_texture(path, color_space));
        update_textures();
        return Handle<Texture>{ textures.size() - 1 };
    }

    Handle<Model> upload_model(const std::string& path) {
        models.emplace_back(load_model(path));

        return Handle<Model>{ models.size() - 1 };
    }

    Handle<Model> upload_model(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const char* diffuse, const char* specular, const char* normal) {
        models.emplace_back(load_model(vertices, indices, diffuse, specular, normal));

        return Handle<Model>{ models.size() - 1 };
    }

    Handle<Texture> upload_texture(const u8 r, const u8 g, const u8 b, const u8 a, const vk::Format color_space) {
        u8 color[]{
            r, g, b ,a
        };

        textures.emplace_back(load_texture(color, 1, 1, 4, color_space));
        update_textures();
        return Handle<Texture>{ textures.size() - 1 };
    }

    static void update_transforms(const RenderData& data) {
        auto& current = transform_buffer[current_frame];

        std::vector<glm::mat4> transforms;
        transforms.reserve(data.draw_commands.size());

        for (const auto& each : data.draw_commands) {
            transforms.emplace_back(each.transform);
        }

        if (current.size() == transforms.size()) {
            current.write(transforms);
        } else {
            current.write(transforms);

            api::SingleUpdateBufferInfo info{}; {
                info.buffer = current.info();
                info.type = vk::DescriptorType::eStorageBuffer;
                info.binding = binding::transform;
            }

            minimal_set[current_frame].update(info);
            shadow_set[current_frame].update(info);
        }
    }

    static void update_camera(const Camera& camera) {
        auto& current = camera_buffer[current_frame];

        if (current.size() == 1) {
            camera_buffer[current_frame].write(camera);
        } else {
            current.write(camera);

            api::SingleUpdateBufferInfo info{}; {
                info.buffer = current.info();
                info.type = vk::DescriptorType::eUniformBuffer;
                info.binding = binding::camera;
            }

            minimal_set[current_frame].update(info);
        }
    }

    static void update_point_lights(const std::vector<PointLight>& point_lights) {
        auto& current = point_light_buffer[current_frame];

        if (point_lights.empty()) {
            return;
        }

        if (current.size() == point_lights.size()) {
            current.write(point_lights);
        } else {
            current.write(point_lights);

            api::SingleUpdateBufferInfo info{}; {
                info.buffer = current.info();
                info.type = vk::DescriptorType::eStorageBuffer;
                info.binding = binding::point_light;
            }

            generic_set[current_frame].update(info);
        }
    }

    static void update_directional_lights(const std::vector<DirectionalLight>& directional_lights) {
        auto& current = directional_light_buffer[current_frame];

        if (directional_lights.empty()) {
            return;
        }

        if (current.size() == directional_lights.size()) {
            current.write(directional_lights);
        } else {
            current.write(directional_lights);

            api::SingleUpdateBufferInfo info{}; {
                info.buffer = current.info();
                info.type = vk::DescriptorType::eStorageBuffer;
                info.binding = binding::directional_light;
            }

            generic_set[current_frame].update(info);
        }
    }

    void unload_geometry(Handle<Mesh>&& mesh) {
        if (mesh.vbo_index < vertex_buffers.size()) {
            std::swap(vertex_buffers[mesh.vbo_index], vertex_buffers.emplace_back());
            api::destroy_buffer(vertex_buffers.back().buffer);
            free_vertex_buffers.push(mesh.vbo_index);
            vertex_buffers.pop_back();
        }

        if (mesh.ibo_index < index_buffers.size()) {
            std::swap(index_buffers[mesh.ibo_index], index_buffers.emplace_back());
            api::destroy_buffer(index_buffers.back().buffer);
            free_index_buffers.push(mesh.ibo_index);
            index_buffers.pop_back();
        }
    }

    static void shadow_depth_draw_pass(const RenderData& data) {
        auto& command_buffer = command_buffers[image_index];

        auto light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -15.0f, 10.0f);

        auto light_view = glm::lookAt(
            data.point_lights[0].position,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        auto light_pv = light_proj * light_view;

        light_space_buffer.write(light_pv);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shadow.handle, ctx.dispatcher);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shadow.layout.pipeline, 0, shadow_set[current_frame].handle(), nullptr, ctx.dispatcher);

        for (usize i = 0; i < data.draw_commands.size(); ++i) {
            auto& draw = data.draw_commands[i];
            auto& model = models[draw.model.index];

            for (const auto& submesh : model.submeshes) {
                auto& vbo = vertex_buffers[submesh.mesh.vbo_index];
                auto& ibo = index_buffers[submesh.mesh.ibo_index];

                std::array indices{
                    static_cast<u32>(i)
                };

                command_buffer.pushConstants<u32>(shadow.layout.pipeline, vk::ShaderStageFlagBits::eVertex, 0, indices, ctx.dispatcher);
                command_buffer.bindIndexBuffer(ibo.buffer.handle, 0, vk::IndexType::eUint32, ctx.dispatcher);
                command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
                command_buffer.drawIndexed(ibo.size, 1, 0, 0, 0, ctx.dispatcher);
            }
        }
    }

    static void final_draw_pass(const RenderData& data) {
        auto& command_buffer = command_buffers[image_index];

        for (usize i = 0; i < data.draw_commands.size(); ++i) {
            auto& draw = data.draw_commands[i];
            auto& model = models[draw.model.index];

            for (const auto& submesh : model.submeshes) {
                auto& vbo = vertex_buffers[submesh.mesh.vbo_index];
                auto& ibo = index_buffers[submesh.mesh.ibo_index];

                if (draw.shader == shader::generic) {
                    std::array<vk::DescriptorSet, 2> sets{
                        minimal_set[current_frame].handle(),
                        generic_set[current_frame].handle()
                    };

                    std::array indices{
                        static_cast<u32>(i),
                        static_cast<u32>(submesh.diffuse.index),
                        static_cast<u32>(submesh.specular.index),
                        static_cast<u32>(submesh.normal.index),
                        static_cast<u32>(data.point_lights.size()),
                        static_cast<u32>(data.directional_lights.size())
                    };

                    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout.pipeline, 0, sets, nullptr, ctx.dispatcher);
                    command_buffer.pushConstants<u32>(generic.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, ctx.dispatcher);
                } else if (draw.shader == shader::minimal) {
                    std::array indices{
                        static_cast<u32>(i),
                        static_cast<u32>(submesh.diffuse.index)
                    };

                    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, minimal.handle, ctx.dispatcher);
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, minimal.layout.pipeline, 0, minimal_set[current_frame].handle(), nullptr, ctx.dispatcher);
                    command_buffer.pushConstants<u32>(minimal.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, ctx.dispatcher);
                }

                command_buffer.bindIndexBuffer(ibo.buffer.handle, 0, vk::IndexType::eUint32, ctx.dispatcher);
                command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
                command_buffer.drawIndexed(ibo.size, 1, 0, 0, 0, ctx.dispatcher);
            }
        }

        std::array<vk::DescriptorSet, 2> sets{
            minimal_set[current_frame].handle(),
            generic_set[current_frame].handle()
        };

        std::array<u32, 6> indices{};

        auto& vbo = vertex_buffers[debug_quad.vbo_index];
        auto& ibo = index_buffers[debug_quad.ibo_index];

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, debug.handle, ctx.dispatcher);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, debug.layout.pipeline, 0, sets, nullptr, ctx.dispatcher);
        command_buffer.pushConstants<u32>(debug.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, ctx.dispatcher);
        command_buffer.bindIndexBuffer(ibo.buffer.handle, 0, vk::IndexType::eUint32, ctx.dispatcher);
        command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
        command_buffer.drawIndexed(ibo.size, 1, 0, 0, 0, ctx.dispatcher);
    }

    static void copy_to_swapchain() {
        auto& command_buffer = command_buffers[image_index];

        vk::ImageCopy copy{}; {
            copy.extent.width = ctx.swapchain.extent.width;
            copy.extent.height = ctx.swapchain.extent.height;
            copy.extent.depth = 1;
            copy.srcOffset = vk::Offset3D{};
            copy.dstOffset = vk::Offset3D{};
            copy.srcSubresource.baseArrayLayer = 0;
            copy.srcSubresource.layerCount = 1;
            copy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            copy.srcSubresource.mipLevel = 0;
            copy.dstSubresource.baseArrayLayer = 0;
            copy.dstSubresource.layerCount = 1;
            copy.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            copy.dstSubresource.mipLevel = 0;
        }

        vk::ImageMemoryBarrier copy_barrier{}; {
            copy_barrier.image = ctx.swapchain.images[image_index];
            copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            copy_barrier.subresourceRange.layerCount = 1;
            copy_barrier.subresourceRange.baseArrayLayer = 0;
            copy_barrier.subresourceRange.levelCount = 1;
            copy_barrier.subresourceRange.baseMipLevel = 0;
            copy_barrier.oldLayout = vk::ImageLayout::eUndefined;
            copy_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            copy_barrier.srcAccessMask = {};
            copy_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        }

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlagBits{},
            nullptr,
            nullptr,
            copy_barrier,
            ctx.dispatcher);

        command_buffer.copyImage(
            offscreen.image.handle, vk::ImageLayout::eTransferSrcOptimal,
            ctx.swapchain.images[image_index], vk::ImageLayout::eTransferDstOptimal,
            copy, ctx.dispatcher);

        vk::ImageMemoryBarrier present_barrier{}; {
            present_barrier.image = ctx.swapchain.images[image_index];
            present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            present_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            present_barrier.subresourceRange.layerCount = 1;
            present_barrier.subresourceRange.baseArrayLayer = 0;
            present_barrier.subresourceRange.levelCount = 1;
            present_barrier.subresourceRange.baseMipLevel = 0;
            present_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            present_barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            present_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            present_barrier.dstAccessMask = {};
        }

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::DependencyFlagBits{},
            nullptr,
            nullptr,
            present_barrier,
            ctx.dispatcher);
    }


    void draw(const RenderData& data) {
        image_index = ctx.device.logical.acquireNextImageKHR(ctx.swapchain.handle, -1, image_available[current_frame], nullptr, ctx.dispatcher).value;

        if (!in_flight[current_frame]) {
            vk::FenceCreateInfo fence_create_info{}; {
                fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
            }

            in_flight[current_frame] = ctx.device.logical.createFence(fence_create_info, nullptr, ctx.dispatcher);
        }

        ctx.device.logical.waitForFences(in_flight[current_frame], true, -1, ctx.dispatcher);

        auto& command_buffer = command_buffers[image_index];

        vk::CommandBufferBeginInfo begin_info{}; {
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }

        command_buffer.begin(begin_info, ctx.dispatcher);

        update_transforms(data);
        update_camera(data.camera);
        update_point_lights(data.point_lights);
        update_directional_lights(data.directional_lights);

        /* Shadow pass */ {
            vk::ClearValue clear{};
            clear.depthStencil = vk::ClearDepthStencilValue{ { 1.0f, 0 } };

            vk::RenderPassBeginInfo render_pass_begin_info{}; {
                render_pass_begin_info.renderArea.extent.width = shadow_depth.image.width;
                render_pass_begin_info.renderArea.extent.height = shadow_depth.image.height;
                render_pass_begin_info.framebuffer = shadow_depth_framebuffer;
                render_pass_begin_info.renderPass = shadow_depth_render_pass;
                render_pass_begin_info.clearValueCount = 1;
                render_pass_begin_info.pClearValues = &clear;
            }

            vk::Viewport viewport{}; {
                viewport.width = shadow_depth.image.width;
                viewport.height = -shadow_depth.image.height;
                viewport.x = 0;
                viewport.y = -viewport.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
            }

            vk::Rect2D scissor{}; {
                scissor.extent.width = shadow_depth.image.width;
                scissor.extent.height = shadow_depth.image.height;
                scissor.offset = { { 0, 0 } };
            }

            command_buffer.setViewport(0, viewport, ctx.dispatcher);
            command_buffer.setScissor(0, scissor, ctx.dispatcher);
            command_buffer.setDepthBias(1.25f, 0.0f, 1.75f, ctx.dispatcher);

            command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline, ctx.dispatcher);
            shadow_depth_draw_pass(data);
            command_buffer.endRenderPass(ctx.dispatcher);
        }

        /* Final color pass */ {
            std::array<vk::ClearValue, 2> clear_values{}; {
                clear_values[0].color = vk::ClearColorValue{ std::array{ 0.01f, 0.01f, 0.01f, 0.0f } };
                clear_values[1].depthStencil = vk::ClearDepthStencilValue{ { 1.0f, 0 } };
            }

            vk::RenderPassBeginInfo render_pass_begin_info{}; {
                render_pass_begin_info.renderArea.extent = ctx.swapchain.extent;
                render_pass_begin_info.framebuffer = offscreen_framebuffer;
                render_pass_begin_info.renderPass = offscreen_render_pass;
                render_pass_begin_info.clearValueCount = clear_values.size();
                render_pass_begin_info.pClearValues = clear_values.data();
            }

            vk::Viewport viewport{}; {
                viewport.width = static_cast<float>(ctx.swapchain.extent.width);
                viewport.height = -static_cast<float>(ctx.swapchain.extent.height);
                viewport.x = 0;
                viewport.y = -viewport.height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
            }

            vk::Rect2D scissor{}; {
                scissor.extent = ctx.swapchain.extent;
                scissor.offset = { { 0, 0 } };
            }

            command_buffer.setViewport(0, viewport, ctx.dispatcher);
            command_buffer.setScissor(0, scissor, ctx.dispatcher);

            command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline, ctx.dispatcher);
            final_draw_pass(data);
            command_buffer.endRenderPass(ctx.dispatcher);
        }

        copy_to_swapchain();

        command_buffer.end(ctx.dispatcher);
    }

    void submit() {
        vk::PipelineStageFlags wait_mask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submit_info{}; {
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffers[image_index];
            submit_info.pWaitDstStageMask = &wait_mask;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &image_available[current_frame];
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &render_finished[current_frame];
        }

        ctx.device.logical.resetFences(in_flight[current_frame], ctx.dispatcher);
        ctx.device.queue.submit(submit_info, in_flight[current_frame], ctx.dispatcher);

        vk::PresentInfoKHR present_info{}; {
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &render_finished[current_frame];
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &ctx.swapchain.handle;
            present_info.pImageIndices = &image_index;
        }

        ctx.device.queue.presentKHR(present_info, ctx.dispatcher);

        current_frame = (current_frame + 1) % frames_in_flight;
    }
} // namespace tethys::renderer