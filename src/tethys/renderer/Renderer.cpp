#include <tethys/api/private/CommandBuffer.hpp>
#include <tethys/api/private/VertexBuffer.hpp>
#include <tethys/api/private/Pipeline.hpp>
#include <tethys/renderer/DrawCommand.hpp>
#include <tethys/api/private/Context.hpp>
#include <tethys/api/private/Buffer.hpp>
#include <tethys/renderer/Renderer.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::renderer {
    constexpr static auto frames_in_flight = 2;

    using namespace api;

    static std::vector<vk::Semaphore> image_available{};
    static std::vector<vk::Semaphore> render_finished{};
    static std::vector<vk::Fence> in_flight{};

    static std::vector<vk::CommandBuffer> command_buffers{};

    static u32 image_index{};
    static u32 current_frame{};

    static api::Pipeline generic{};

    static std::vector<api::Buffer> vertex_buffers;

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

        generic = api::make_generic_pipeline("shaders/generic.vert.spv", "shaders/generic.frag.spv");
    }

    Handle<Mesh> upload(Mesh&& mesh) {
        static usize index = 0;

        vertex_buffers.emplace_back(api::make_vertex_buffer(std::move(mesh.geometry)));

        return Handle<Mesh>{ index++ };
    }

    std::vector<Handle<Mesh>> upload(std::vector<Mesh>&& meshes) {
        std::vector<Handle<Mesh>> mesh_handles;

        mesh_handles.reserve(meshes.size());

        for (auto&& each : meshes) {
            mesh_handles.emplace_back(upload(std::move(each)));
        }

        return mesh_handles;
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

    void draw(const std::vector<DrawCommand>& draws) {
        auto& command_buffer = command_buffers[image_index];

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, ctx.dispatcher);

        for (const auto& draw : draws) {
            auto& mesh = vertex_buffers[draw.mesh.index];

            command_buffer.bindVertexBuffers(0, mesh.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
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