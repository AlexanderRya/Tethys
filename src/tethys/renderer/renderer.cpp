#include <tethys/api/command_buffer.hpp>
#include <tethys/api/descriptor_set.hpp>
#include <tethys/renderer/renderer.hpp>
#include <tethys/directional_light.hpp>
#include <tethys/api/vertex_buffer.hpp>
#include <tethys/api/static_buffer.hpp>
#include <tethys/api/index_buffer.hpp>
#include <tethys/api/render_pass.hpp>
#include <tethys/api/framebuffer.hpp>
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

#include <glm/mat4x4.hpp>

#include <vector>
#include <stack>

namespace tethys::renderer {
    using namespace api;

    static vk::RenderPass offscreen_render_pass{};
    static vk::Framebuffer offscreen_framebuffer{};

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

    static void update_textures() {
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
    }

    void initialise() {
        offscreen_render_pass = api::make_offscreen_render_pass();
        offscreen_framebuffer = api::make_offscreen_framebuffer(offscreen_render_pass);
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
            generic_info.render_pass = offscreen_render_pass;
            generic_info.layout_idx = layout::generic;
        }

        generic = make_pipeline(generic_info);

        Pipeline::CreateInfo minimal_info{}; {
            minimal_info.vertex = "shaders/minimal.vert.spv";
            minimal_info.fragment = "shaders/minimal.frag.spv";
            minimal_info.subpass_idx = 0;
            minimal_info.render_pass = offscreen_render_pass;
            minimal_info.layout_idx = layout::minimal;
        }
        minimal = make_pipeline(minimal_info);

        camera_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);
        transform_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        point_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
        directional_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);

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

        std::array<u8, 4> white = { 255, 255, 255, 255 };

        textures.emplace_back(load_texture(white.data(), 1, 1, 4, vk::Format::eR8G8B8A8Srgb));

        std::array<u8, 4> black{};

        textures.emplace_back(load_texture(black.data(), 1, 1, 4, vk::Format::eR8G8B8A8Srgb));
        update_textures();
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

        std::array<vk::ClearValue, 2> clear_values{}; {
            clear_values[0].color = vk::ClearColorValue{ std::array{ 0.001f, 0.001f, 0.001f, 0.0f } };
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

        update_transforms(data);
        update_camera(data.camera);
        update_point_lights(data.point_lights);
        update_directional_lights(data.directional_lights);

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

        command_buffer.endRenderPass(ctx.dispatcher);

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
            copy_barrier.subresourceRange.baseArrayLayer = 0;
            copy_barrier.subresourceRange.layerCount = 1;
            copy_barrier.subresourceRange.levelCount = 1;
            copy_barrier.subresourceRange.baseMipLevel = 0;
            copy_barrier.oldLayout = vk::ImageLayout::eUndefined;
            copy_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            copy_barrier.srcAccessMask = {};
            copy_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        }

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            vk::DependencyFlagBits{},
            nullptr,
            nullptr,
            copy_barrier,
            ctx.dispatcher);

        command_buffer.copyImage(
            ctx.offscreen.image.handle, vk::ImageLayout::eTransferSrcOptimal,
            ctx.swapchain.images[image_index], vk::ImageLayout::eTransferDstOptimal,
            copy, ctx.dispatcher);

        vk::ImageMemoryBarrier present_barrier{}; {
            present_barrier.image = ctx.swapchain.images[image_index];
            present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            present_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            present_barrier.subresourceRange.baseArrayLayer = 0;
            present_barrier.subresourceRange.layerCount = 1;
            present_barrier.subresourceRange.levelCount = 1;
            present_barrier.subresourceRange.baseMipLevel = 0;
            present_barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            present_barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            present_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            present_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
        }

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eAllGraphics,
            vk::DependencyFlagBits{},
            nullptr,
            nullptr,
            present_barrier,
            ctx.dispatcher);

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