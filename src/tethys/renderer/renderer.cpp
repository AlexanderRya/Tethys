#include <tethys/api/command_buffer.hpp>
#include <tethys/api/descriptor_set.hpp>
#include <tethys/api/vertex_buffer.hpp>
#include <tethys/api/static_buffer.hpp>
#include <tethys/renderer/renderer.hpp>
#include <tethys/directional_light.hpp>
#include <tethys/api/index_buffer.hpp>
#include <tethys/point_light.hpp>
#include <tethys/render_data.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/buffer.hpp>
#include <tethys/constants.hpp>
#include <tethys/forwards.hpp>
#include <tethys/pipeline.hpp>
#include <tethys/texture.hpp>
#include <tethys/acquire.hpp>
#include <tethys/handle.hpp>
#include <tethys/model.hpp>
#include <tethys/types.hpp>
#include <tethys/mesh.hpp>

#include <vulkan/vulkan.hpp>

#include <glm/mat4x4.hpp>

#include <vector>
#include <stack>

namespace tethys::renderer {
    using namespace api;

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

    static std::vector<Texture> textures{};

    static std::vector<Model> models{};

    static api::Buffer<Camera> camera_buffer{};
    static api::Buffer<glm::mat4> transform_buffer{};
    static api::Buffer<PointLight> point_light_buffer{};
    static api::Buffer<DirectionalLight> directional_light_buffer{};

    static api::DescriptorSet generic_set{};
    static api::DescriptorSet minimal_set{};

    static Pipeline generic;
    static Pipeline minimal;

    void initialise() {
        command_buffers = api::make_rendering_command_buffers();

        vk::SemaphoreCreateInfo semaphore_create_info{};

        image_available.reserve(frames_in_flight);
        render_finished.reserve(frames_in_flight);

        for (u64 i = 0; i < frames_in_flight; ++i) {
            image_available.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
            render_finished.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
        }

        in_flight.resize(frames_in_flight, nullptr);

        load_all_builtin_shaders();

        camera_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);
        transform_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        point_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        directional_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);

        generic = acquire<Pipeline>(shader::generic);
        minimal = acquire<Pipeline>(shader::minimal);

        generic_set.create(acquire<vk::DescriptorSetLayout>(layout::generic));
        minimal_set.create(acquire<vk::DescriptorSetLayout>(layout::minimal));

        std::vector<api::UpdateBufferInfo> minimal_update(2); {
            minimal_update[0].binding = binding::camera;
            minimal_update[0].type = vk::DescriptorType::eUniformBuffer;
            minimal_update[0].buffers = camera_buffer.info();

            minimal_update[1].binding = binding::transform;
            minimal_update[1].type = vk::DescriptorType::eStorageBuffer;
            minimal_update[1].buffers = transform_buffer.info();
        }

        minimal_set.update(minimal_update);

        std::vector<api::UpdateBufferInfo> generic_update(2); {
            generic_update[0].binding = binding::point_light;
            generic_update[0].type = vk::DescriptorType::eStorageBuffer;
            generic_update[0].buffers = point_light_buffer.info();

            generic_update[1].binding = binding::directional_light;
            generic_update[1].type = vk::DescriptorType::eStorageBuffer;
            generic_update[1].buffers = directional_light_buffer.info();
        }

        generic_set.update(generic_update);

        const u8 white_texture[]{ 255, 255, 255, 255 };

        api::UpdateImageInfo info{}; {
            info.images = { textures.emplace_back(load_texture(white_texture, 1, 1, 4)).info() };
            info.binding = binding::texture;
            info.type = vk::DescriptorType::eCombinedImageSampler;
        }

        minimal_set.update(info);
    }

    template <>
    Handle<Mesh> upload<Mesh>(std::vector<Vertex>&& geometry, std::vector<u32>&& indices) {
        usize vbo_index, ibo_index;

        if (!free_vertex_buffers.empty()) {
            vbo_index = free_vertex_buffers.top();
            vertex_buffers[vbo_index] = api::make_vertex_buffer(std::move(geometry));
            free_vertex_buffers.pop();
        } else {
            vertex_buffers.emplace_back(api::make_vertex_buffer(std::move(geometry)));
            vbo_index = vertex_buffers.size() - 1;
        }

        if (!free_index_buffers.empty()) {
            ibo_index = free_index_buffers.top();
            index_buffers[ibo_index] = api::make_index_buffer(std::move(indices));
            free_index_buffers.pop();
        } else {
            index_buffers.emplace_back(api::make_index_buffer(std::move(indices)));
            ibo_index = index_buffers.size() - 1;
        }

        return Handle<Mesh>{ vbo_index, ibo_index };
    }

    template <>
    Handle<Texture> upload<Texture>(const char* path) {
        textures.emplace_back(load_texture(path));

        std::vector<vk::DescriptorImageInfo> images{};
        images.reserve(textures.size());

        for (const auto& image : textures) {
            images.emplace_back(image.info());
        }

        api::UpdateImageInfo info{}; {
            info.images = std::move(images);
            info.binding = binding::texture;
            info.type = vk::DescriptorType::eCombinedImageSampler;
        }

        minimal_set.update(info);

        return Handle<Texture>{ textures.size() - 1 };
    }

    template <>
    Handle<Model> upload<Model>(const char* path) {
        models.emplace_back(load_model(path));

        return Handle<Model>{ models.size() - 1 };
    }

    static void update_transforms(const RenderData& data) {
        auto& current = transform_buffer[current_frame];

        std::vector<glm::mat4> transforms;
        transforms.reserve(data.mesh_commands.size() + data.model_commands.size());

        for (const auto& each : data.mesh_commands) {
            transforms.emplace_back(each.transform);
        }

        for (const auto& each : data.model_commands) {
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

    void unload(Handle<Mesh>&& mesh) {
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

    static void acquire() {
        image_index = ctx.device.logical.acquireNextImageKHR(ctx.swapchain.handle, -1, image_available[current_frame], nullptr, ctx.dispatcher).value;

        if (!in_flight[current_frame]) {
            vk::FenceCreateInfo fence_create_info{}; {
                fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
            }

            in_flight[current_frame] = ctx.device.logical.createFence(fence_create_info, nullptr, ctx.dispatcher);
        }

        ctx.device.logical.waitForFences(in_flight[current_frame], true, -1, ctx.dispatcher);
    }

    void start() {
        acquire();

        auto& command_buffer = command_buffers[image_index];

        vk::CommandBufferBeginInfo begin_info{}; {
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }

        command_buffer.begin(begin_info, ctx.dispatcher);

        std::array<vk::ClearValue, 2> clear_values{}; {
            clear_values[0].color = vk::ClearColorValue{ std::array{ 0.01f, 0.01f, 0.01f, 0.0f } };
            clear_values[1].depthStencil = vk::ClearDepthStencilValue{ { 1.0f, 0 } };
        }

        vk::RenderPassBeginInfo render_pass_begin_info{}; {
            render_pass_begin_info.renderArea.extent = ctx.swapchain.extent;
            render_pass_begin_info.framebuffer = ctx.default_framebuffers[image_index];
            render_pass_begin_info.renderPass = ctx.default_render_pass;
            render_pass_begin_info.clearValueCount = clear_values.size();
            render_pass_begin_info.pClearValues = clear_values.data();
        }

        vk::Viewport viewport{}; {
            viewport.width = ctx.swapchain.extent.width;
            viewport.height = ctx.swapchain.extent.height;
            viewport.x = 0;
            viewport.y = 0;
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
    }

    void draw(const RenderData& data) {
        auto& command_buffer = command_buffers[image_index];

        update_transforms(data);
        update_camera(data.camera);
        update_point_lights(data.point_lights);
        update_directional_lights(data.directional_lights);

        for (usize i = 0; i < data.mesh_commands.size(); ++i) {
            auto& draw = data.mesh_commands[i];

            auto& vbo = vertex_buffers[draw.mesh.vbo_index];
            auto& ibo = index_buffers[draw.mesh.ibo_index];

            u32 indices[]{
                static_cast<u32>(i),
                static_cast<u32>(draw.material.texture.index),
                static_cast<u32>(texture::white),
                static_cast<u32>(texture::white)
            };

            if (draw.material.shader == shader::generic) {
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);
                std::array<vk::DescriptorSet, 2> sets{
                    minimal_set[current_frame].handle(),
                    generic_set[current_frame].handle()
                };
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout.pipeline, 0, sets, nullptr, ctx.dispatcher);
                command_buffer.pushConstants<const u32*>(generic.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), indices, ctx.dispatcher);
            } else if (draw.material.shader == shader::minimal) {
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, minimal.handle, ctx.dispatcher);
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, minimal.layout.pipeline, 0, minimal_set[current_frame].handle(), nullptr, ctx.dispatcher);
                command_buffer.pushConstants<const u32*>(minimal.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), indices, ctx.dispatcher);
            }

            command_buffer.bindIndexBuffer(ibo.buffer.handle, static_cast<vk::DeviceSize>(0), vk::IndexType::eUint32, ctx.dispatcher);
            command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
            command_buffer.draw(vbo.size, 1, 0, 0, ctx.dispatcher);
        }

        for (usize i = 0; i < data.model_commands.size(); ++i) {
            auto& draw = data.model_commands[i];
            auto& model = models[draw.model.index];

            for (auto& submesh : model.submeshes) {
                auto& vbo = vertex_buffers[submesh.mesh.vbo_index];
                auto& ibo = index_buffers[submesh.mesh.ibo_index];

                u32 indices[]{
                    static_cast<u32>(data.mesh_commands.size() + i),
                    static_cast<u32>(submesh.diffuse.index),
                    static_cast<u32>(submesh.specular.index),
                    static_cast<u32>(submesh.normal.index)
                };

                if (draw.shader == shader::generic) {
                    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);
                    std::array<vk::DescriptorSet, 2> sets{
                        minimal_set[current_frame].handle(),
                        generic_set[current_frame].handle()
                    };
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout.pipeline, 0, sets, nullptr, ctx.dispatcher);
                    command_buffer.pushConstants<const u32*>(generic.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), indices, ctx.dispatcher);
                } else if (draw.shader == shader::minimal) {
                    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, minimal.handle, ctx.dispatcher);
                    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, minimal.layout.pipeline, 0, minimal_set[current_frame].handle(), nullptr, ctx.dispatcher);
                    command_buffer.pushConstants<const u32*>(minimal.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), indices, ctx.dispatcher);
                }

                command_buffer.bindIndexBuffer(ibo.buffer.handle, static_cast<vk::DeviceSize>(0), vk::IndexType::eUint32, ctx.dispatcher);
                command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
                command_buffer.draw(vbo.size, 1, 0, 0, ctx.dispatcher);
            }
        }
    }

    void end() {
        auto& command_buffer = command_buffers[image_index];

        command_buffer.endRenderPass(ctx.dispatcher);

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
