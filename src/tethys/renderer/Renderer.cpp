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

    struct RendererContext {
        std::vector<vk::Semaphore> image_available{};
        std::vector<vk::Semaphore> render_finished{};
        std::vector<vk::Fence> in_flight{};

        std::vector<vk::CommandBuffer> command_buffers{};

        u32 image_index{};
        u32 current_frame{};

        api::Pipeline generic{};

        std::vector<api::Buffer> vertex_buffers;
    } renderer;

    void initialise() {
        renderer.command_buffers = api::make_rendering_command_buffers();

        vk::SemaphoreCreateInfo semaphore_create_info{};

        renderer.image_available.reserve(frames_in_flight);
        renderer.render_finished.reserve(frames_in_flight);

        for (u64 i = 0; i < frames_in_flight; ++i) {
            renderer.image_available.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
            renderer.render_finished.emplace_back(ctx.device.logical.createSemaphore(semaphore_create_info, nullptr, ctx.dispatcher));
        }

        renderer.in_flight.resize(frames_in_flight, nullptr);

        renderer.generic = api::make_generic_pipeline("shaders/generic.vert.spv", "shaders/generic.frag.spv");
    }

    Handle<Mesh> upload(Mesh&& mesh) {
        static usize index = 0;

        renderer.vertex_buffers.emplace_back(api::make_vertex_buffer(std::move(mesh.geometry)));

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
        renderer.image_index = ctx.device.logical.acquireNextImageKHR(ctx.swapchain.handle, -1, renderer.image_available[renderer.current_frame], nullptr, ctx.dispatcher).value;

        if (!renderer.in_flight[renderer.current_frame]) {
            vk::FenceCreateInfo fence_create_info{}; {
                fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
            }

            renderer.in_flight[renderer.current_frame] = ctx.device.logical.createFence(fence_create_info, nullptr, ctx.dispatcher);
        }

        ctx.device.logical.waitForFences(renderer.in_flight[renderer.current_frame], true, -1, ctx.dispatcher);
    }

    void start() {
        acquire();

        auto& command_buffer = renderer.command_buffers[renderer.image_index];

        vk::CommandBufferBeginInfo begin_info{}; {
            begin_info.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }

        command_buffer.begin(begin_info, ctx.dispatcher);

        std::array<vk::ClearValue, 2> clear_values{}; {
            clear_values[0].color = vk::ClearColorValue{ std::array{ 0.01f, 0.01f, 0.01f, 0.0f } };
            clear_values[1].depthStencil = vk::ClearDepthStencilValue{ { 1.0f, 0 } };
        }

        vk::RenderPassBeginInfo render_pass_begin_info{}; {
            render_pass_begin_info.renderArea.extent = ctx.swapchain.extent;
            render_pass_begin_info.framebuffer = ctx.default_framebuffers[renderer.image_index];
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
        auto& command_buffer = renderer.command_buffers[renderer.image_index];

        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderer.generic.handle, ctx.dispatcher);

        for (const auto& draw : draws) {
            auto& mesh = renderer.vertex_buffers[draw.mesh.index];

            command_buffer.bindVertexBuffers(0, mesh.handle, static_cast<vk::DeviceSize>(0), ctx.dispatcher);
            command_buffer.draw(mesh.size, 1, 0, 0, ctx.dispatcher);
        }
    }

    void end() {
        auto& command_buffer = renderer.command_buffers[renderer.image_index];

        command_buffer.endRenderPass(ctx.dispatcher);

        command_buffer.end(ctx.dispatcher);
    }

    void submit() {
        vk::PipelineStageFlags wait_mask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submit_info{}; {
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &renderer.command_buffers[renderer.image_index];
            submit_info.pWaitDstStageMask = &wait_mask;
            submit_info.waitSemaphoreCount = 1;
            submit_info.pWaitSemaphores = &renderer.image_available[renderer.current_frame];
            submit_info.signalSemaphoreCount = 1;
            submit_info.pSignalSemaphores = &renderer.render_finished[renderer.current_frame];
        }

        ctx.device.logical.resetFences(renderer.in_flight[renderer.current_frame], ctx.dispatcher);
        ctx.device.queue.submit(submit_info, renderer.in_flight[renderer.current_frame], ctx.dispatcher);

        vk::PresentInfoKHR present_info{}; {
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &renderer.render_finished[renderer.current_frame];
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &ctx.swapchain.handle;
            present_info.pImageIndices = &renderer.image_index;
        }

        ctx.device.queue.presentKHR(present_info, ctx.dispatcher);

        renderer.current_frame = (renderer.current_frame + 1) % frames_in_flight;
    }
} // namespace tethys::renderer