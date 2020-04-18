#include <tethys/api/private/command_buffer.hpp>
#include <tethys/api/private/descriptor_set.hpp>
#include <tethys/api/private/vertex_buffer.hpp>
#include <tethys/api/private/static_buffer.hpp>
#include <tethys/api/private/context.hpp>
#include <tethys/api/private/buffer.hpp>
#include <tethys/renderer/renderer.hpp>
#include <tethys/point_light.hpp>
#include <tethys/render_data.hpp>
#include <tethys/constants.hpp>
#include <tethys/forwards.hpp>
#include <tethys/pipeline.hpp>
#include <tethys/texture.hpp>
#include <tethys/types.hpp>

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

    static std::vector<Texture> textures{};

    static api::Buffer<Camera> camera_buffer{};
    static api::Buffer<glm::mat4> transform_buffer{};
    static api::Buffer<PointLight> point_light_buffer{};

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

        api::UpdateBufferInfo generic_update{}; {
            generic_update.binding = binding::point_light;
            generic_update.type = vk::DescriptorType::eStorageBuffer;
            generic_update.buffers = point_light_buffer.info();
        }

        generic_set.update(generic_update);
    }

    Handle<Mesh> upload(std::vector<Vertex>&& mesh) {
        if (!free_vertex_buffers.empty()) {
            auto free_idx = free_vertex_buffers.top();

            vertex_buffers[free_idx] = api::make_vertex_buffer(std::move(mesh));

            free_vertex_buffers.pop();

            return Handle<Mesh>{ free_idx };
        } else {
            vertex_buffers.emplace_back(api::make_vertex_buffer(std::move(mesh)));

            return Handle<Mesh>{ vertex_buffers.size() - 1 };
        }
    }

    Handle<Texture> upload(const char* path) {
        textures.emplace_back().load(path);

        std::vector<vk::DescriptorImageInfo> image_info{};
        image_info.reserve(textures.size());

        for (const auto& texture : textures) {
            image_info.emplace_back(texture.info());
        }

        api::UpdateImageInfo info{}; {
            info.image = std::move(image_info);
            info.binding = binding::texture;
            info.type = vk::DescriptorType::eCombinedImageSampler;
        }

        minimal_set.update(info);

        return Handle<Texture>{ textures.size() - 1 };
    }

    static inline void update_transforms(const RenderData& data) {
        auto& current = transform_buffer[current_frame];

        std::vector<glm::mat4> transforms;
        transforms.reserve(data.commands.size());

        for (const auto& each : data.commands) {
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

    static inline void update_camera(const Camera& camera) {
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

    static inline void update_point_lights(const std::vector<PointLight>& point_lights) {
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

    void unload(Handle<Mesh>&& mesh) {
        if (mesh.index < vertex_buffers.size()) {
            std::swap(vertex_buffers[mesh.index], vertex_buffers.emplace_back());
            api::destroy_buffer(vertex_buffers.back().buffer);
            free_vertex_buffers.push(mesh.index);
            vertex_buffers.pop_back();
        }
    }

    static inline void acquire() {
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

        for (usize i = 0; i < data.commands.size(); ++i) {
            auto& draw = data.commands[i];

            auto& mesh = vertex_buffers[draw.mesh.index];

            i32 indices[]{
                static_cast<i32>(i),
                static_cast<i32>(draw.material.texture.index)
            };

            if (draw.material.shader == shader::generic) {
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);
                std::array<vk::DescriptorSet, 2> sets{
                    minimal_set[current_frame].handle(),
                    generic_set[current_frame].handle()
                };
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout.pipeline, 0, sets, nullptr, ctx.dispatcher);
                command_buffer.pushConstants(generic.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), vk::ArrayProxy<const i32>{ 2, indices }, ctx.dispatcher);
            } else if (draw.material.shader == shader::minimal) {
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, minimal.handle, ctx.dispatcher);
                command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, minimal.layout.pipeline, 0, minimal_set[current_frame].handle(), nullptr, ctx.dispatcher);
                command_buffer.pushConstants(minimal.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), vk::ArrayProxy<const i32>{ 2, indices }, ctx.dispatcher);
            }
            command_buffer.bindVertexBuffers(0, mesh.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
            command_buffer.draw(mesh.size, 1, 0, 0, ctx.dispatcher);
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
