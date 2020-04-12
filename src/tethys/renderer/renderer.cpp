#include <tethys/api/private/command_buffer.hpp>
#include <tethys/api/private/descriptor_set.hpp>
#include <tethys/api/private/vertex_buffer.hpp>
#include <tethys/api/private/static_buffer.hpp>
#include <tethys/api/private/pipeline.hpp>
#include <tethys/renderer/render_data.hpp>
#include <tethys/api/private/context.hpp>
#include <tethys/api/meta/constants.hpp>
#include <tethys/api/private/buffer.hpp>
#include <tethys/renderer/renderer.hpp>
#include <tethys/forwards.hpp>
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

    static api::Pipeline generic{};

    static std::stack<usize> free_vertex_buffers{};
    static std::vector<api::VertexBuffer> vertex_buffers{};

    static std::vector<Texture> textures{};

    static api::Buffer<glm::mat4> camera_buffer{};
    static api::Buffer<glm::mat4> transform_buffer{};

    static api::DescriptorSet descriptor_set{};

    void initialise() {
        command_buffers = api::make_rendering_command_buffers();

        vk::SemaphoreCreateInfo semaphore_create_info{};

        image_available.reserve(meta::frames_in_flight);
        render_finished.reserve(meta::frames_in_flight);

        for (u64 i = 0; i < meta::frames_in_flight; ++i) {
            image_available.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
            render_finished.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
        }

        in_flight.resize(meta::frames_in_flight, nullptr);

        generic = api::make_generic_pipeline("shaders/generic.vert.spv", "shaders/generic.frag.spv");

        descriptor_set.create(generic.layout.set);

        camera_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);
        transform_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);

        std::vector<api::UpdateBufferInfo> info(2); {
            info[0].binding = meta::binding::camera;
            info[0].type = vk::DescriptorType::eUniformBuffer;
            info[0].buffers = camera_buffer.info();
        }

        descriptor_set.update(info);
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

    std::vector<Handle<Mesh>> upload(std::vector<std::vector<Vertex>>&& meshes) {
        std::vector<Handle<Mesh>> mesh_handles;

        mesh_handles.reserve(meshes.size());

        for (auto&& each : meshes) {
            mesh_handles.emplace_back(upload(std::move(each)));
        }

        return mesh_handles;
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
            info.binding = meta::binding::texture;
            info.type = vk::DescriptorType::eCombinedImageSampler;
        }

        descriptor_set[current_frame].update(info);

        return Handle<Texture>{ textures.size() - 1 };
    }

    static inline void update_transforms(const RenderData& data) {
        auto& current = transform_buffer[current_frame];

        std::vector<glm::mat4> transforms;
        transforms.reserve(data.commands.size());

        for (const auto& each : data.commands) {
            transforms.emplace_back(each.transform);
        }

        if (current.size() != transforms.size()) {
            current.write(transforms);
        } else {
            current.write(transforms);

            api::SingleUpdateBufferInfo info{}; {
                info.buffer = current.info();
                info.type = vk::DescriptorType::eStorageBuffer;
                info.binding = meta::binding::transform;
            }

            descriptor_set[current_frame].update(info);
        }
    }

    static inline void update_camera(const glm::mat4& pvmat) {
        camera_buffer[current_frame].write(pvmat);
    }

    void unload(Handle<Mesh>&& mesh) {
        if (mesh.index < vertex_buffers.size()) {
            std::swap(vertex_buffers[mesh.index], vertex_buffers.emplace_back());
            api::destroy_buffer(vertex_buffers.back().buffer);
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
        update_camera(data.pv_matrix);

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);
        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout.pipeline, 0, descriptor_set[current_frame].handle(), nullptr, ctx.dispatcher);

        for (usize i = 0; i < data.commands.size(); ++i) {
            auto& draw = data.commands[i];

            auto& mesh = vertex_buffers[draw.mesh.index];

            i32 indices[]{
                static_cast<i32>(i),
                static_cast<i32>(draw.texture.index)
            };

            command_buffer.bindVertexBuffers(0, mesh.buffer.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
            command_buffer.pushConstants<i32*>(generic.layout.pipeline, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, static_cast<vk::DeviceSize>(0), indices, ctx.dispatcher);
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

        current_frame = (current_frame + 1) % meta::frames_in_flight;
    }
} // namespace tethys::renderer
