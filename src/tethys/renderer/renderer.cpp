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
#include <tethys/model.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <stack>
#include <mutex>

namespace tethys {
    namespace renderer {
        static auto& context = api::context;

        static vk::RenderPass offscreen_render_pass{};
        static vk::Framebuffer offscreen_framebuffer{};

        static std::vector<vk::Semaphore> image_available{};
        static std::vector<vk::Semaphore> render_finished{};
        static std::vector<vk::Fence> in_flight{};

        static std::vector<vk::CommandBuffer> command_buffers{};

        static u32 image_index{};
        static u32 current_frame{};

        static api::Offscreen offscreen{};

        // Part of set 0
        static api::Buffer<Camera> camera_buffer{};
        static api::Buffer<glm::mat4> transform_buffer{};
        // Part of set 1
        static api::Buffer<PointLight> point_light_buffer{};
        static api::Buffer<DirectionalLight> directional_light_buffer{};

        static api::DescriptorSet generic_set{};
        static api::DescriptorSet minimal_set{};

        static Pipeline minimal;
        static Pipeline generic;
        static Pipeline pbr;

        static std::vector<Texture> builtin_textures{};
        static std::vector<vk::DescriptorImageInfo> texture_descriptors{};

        static void update_textures() {
            api::UpdateImageInfo info{}; {
                info.image = texture_descriptors.data();
                info.size = texture_descriptors.size();
                info.type = vk::DescriptorType::eCombinedImageSampler;
                info.binding = binding::texture;
            }
            minimal_set.update(info);
        }

        void initialise() {
            offscreen = api::make_offscreen_target();
            offscreen_render_pass = api::make_offscreen_render_pass(offscreen);
            offscreen_framebuffer = api::make_offscreen_framebuffer(offscreen, offscreen_render_pass);

            command_buffers = api::make_rendering_command_buffers();

            vk::SemaphoreCreateInfo semaphore_create_info{};

            image_available.reserve(api::frames_in_flight);
            render_finished.reserve(api::frames_in_flight);

            for (u64 i = 0; i < api::frames_in_flight; ++i) {
                image_available.emplace_back(context.device.logical.createSemaphore(semaphore_create_info, nullptr, context.dispatcher));
                render_finished.emplace_back(context.device.logical.createSemaphore(semaphore_create_info, nullptr, context.dispatcher));
            }

            in_flight.resize(api::frames_in_flight, nullptr);

            layout::load();

            Pipeline::CreateInfo minimal_info{}; {
                minimal_info.vertex = "shaders/minimal.vert.spv";
                minimal_info.fragment = "shaders/minimal.frag.spv";
                minimal_info.subpass_idx = 0;
                minimal_info.render_pass = offscreen_render_pass;
                minimal_info.samples = context.device.samples;
                minimal_info.cull = vk::CullModeFlagBits::eNone;
                minimal_info.dynamic_states = {
                    vk::DynamicState::eViewport,
                    vk::DynamicState::eScissor
                };
                minimal_info.layouts = {
                    layout::get<layout::minimal>()
                };
                minimal_info.push_constants = {
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(u32) * 2
                };
            }
            minimal = make_pipeline(minimal_info);

            Pipeline::CreateInfo generic_info{}; {
                generic_info.vertex = "shaders/generic.vert.spv";
                generic_info.fragment = "shaders/generic.frag.spv";
                generic_info.subpass_idx = 0;
                generic_info.render_pass = offscreen_render_pass;
                generic_info.samples = context.device.samples;
                generic_info.cull = vk::CullModeFlagBits::eNone;
                generic_info.dynamic_states = {
                    vk::DynamicState::eViewport,
                    vk::DynamicState::eScissor
                };
                generic_info.layouts = {
                    layout::get<layout::minimal>(),
                    layout::get<layout::generic>()
                };
                generic_info.push_constants = {
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(u32) * 6
                };
            }
            generic = make_pipeline(generic_info);

            Pipeline::CreateInfo pbr_info{}; {
                pbr_info.vertex = "shaders/pbr.vert.spv";
                pbr_info.fragment = "shaders/pbr.frag.spv";
                pbr_info.subpass_idx = 0;
                pbr_info.render_pass = offscreen_render_pass;
                pbr_info.samples = context.device.samples;
                pbr_info.cull = vk::CullModeFlagBits::eNone;
                pbr_info.dynamic_states = {
                    vk::DynamicState::eViewport,
                    vk::DynamicState::eScissor
                };
                pbr_info.layouts = {
                    layout::get<layout::minimal>(),
                    layout::get<layout::generic>()
                };
                pbr_info.push_constants = {
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                    sizeof(u32) * 8
                };
            }
            pbr = make_pipeline(pbr_info);

            camera_buffer.create(vk::BufferUsageFlagBits::eUniformBuffer);
            transform_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
            point_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);
            directional_light_buffer.create(vk::BufferUsageFlagBits::eStorageBuffer);

            generic_set.create(layout::get<layout::generic>());
            minimal_set.create(layout::get<layout::minimal>());

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

            builtin_textures.reserve(3);
            texture_descriptors.reserve(3);
            builtin_textures.emplace_back(upload_texture(255, 255, 255, 255, vk::Format::eR8G8B8A8Srgb));
            builtin_textures.emplace_back(upload_texture(0, 0, 0, 255, vk::Format::eR8G8B8A8Unorm));
            builtin_textures.emplace_back(upload_texture(0, 255, 0, 255, vk::Format::eR8G8B8A8Unorm));
        }

        Mesh write_geometry(const VertexData& data) {
            return write_geometry(data.geometry, data.indices);
        }

        Mesh write_geometry(const std::vector<Vertex>& geometry, const std::vector<u32>& indices) {
            Mesh mesh{}; {
                mesh.vertex_count = geometry.size();
                mesh.index_count = indices.size();
                mesh.vbo = api::make_vertex_buffer(geometry);
                mesh.ibo = api::make_index_buffer(indices);
            }

            return mesh;
        }

        Model upload_model(const std::string& path) {
            return load_model(path);
        }

        Model upload_model_pbr(const std::string& path) {
            return load_model_pbr(path);
        }

        Model upload_model(const VertexData& data, const char* albedo, const char* metallic, const char* normal) {
            return load_model(data, albedo, metallic, normal);
        }

        Model upload_model(const VertexData& data, const char* albedo, const char* metallic, const char* normal, const char* roughness, const char* occlusion) {
            return load_model(data, albedo, metallic, normal, roughness, occlusion);
        }

        Texture upload_texture(const char* path, const vk::Format color_space) {
            auto texture = load_texture(path, color_space);

            texture_descriptors.emplace_back(texture.info(api::SamplerType::eDefault));
            update_textures();

            return texture;
        }

        Texture upload_texture(const u8 r, const u8 g, const u8 b, const u8 a, const vk::Format color_space) {
            const u8 data[]{
                r, g, b, a
            };
            auto texture = load_texture(data, 1, 1, 4, color_space);

            texture_descriptors.emplace_back(texture.info(api::SamplerType::eDefault));
            update_textures();

            return texture;
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
                current.write(camera);
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

        static void final_draw_pass(const RenderData& data) {
            auto& command_buffer = command_buffers[image_index];

            for (usize i = 0; i < data.draw_commands.size(); ++i) {
                auto& draw = data.draw_commands[i];
                auto& model = draw.model;

                for (const auto& submesh : model.submeshes) {
                    auto& vbo = submesh.mesh.vbo;
                    auto& ibo = submesh.mesh.ibo;
                    auto& index_count = submesh.mesh.index_count;

                    if (draw.shader.handle == generic.handle) {
                        std::array sets{
                            minimal_set[current_frame].handle(),
                            generic_set[current_frame].handle()
                        };

                        std::array indices{
                            static_cast<u32>(i),
                            static_cast<u32>(submesh.albedo.index),
                            static_cast<u32>(submesh.metallic.index),
                            static_cast<u32>(submesh.normal.index),
                            static_cast<u32>(data.point_lights.size()),
                            static_cast<u32>(data.directional_lights.size())
                        };

                        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, generic.handle, context.dispatcher);
                        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, generic.layout, 0, sets, nullptr, context.dispatcher);
                        command_buffer.pushConstants<u32>(generic.layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, context.dispatcher);
                    } else if (draw.shader.handle == minimal.handle) {
                        std::array indices{
                            static_cast<u32>(i),
                            static_cast<u32>(submesh.albedo.index)
                        };

                        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, minimal.handle, context.dispatcher);
                        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, minimal.layout, 0, minimal_set[current_frame].handle(), nullptr, context.dispatcher);
                        command_buffer.pushConstants<u32>(minimal.layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, context.dispatcher);
                    } else if (draw.shader.handle == pbr.handle) {
                        std::array sets{
                            minimal_set[current_frame].handle(),
                            generic_set[current_frame].handle()
                        };

                        std::array indices{
                            static_cast<u32>(i),
                            static_cast<u32>(submesh.albedo.index),
                            static_cast<u32>(submesh.metallic.index),
                            static_cast<u32>(submesh.normal.index),
                            static_cast<u32>(submesh.roughness.index),
                            static_cast<u32>(submesh.occlusion.index),
                            static_cast<u32>(data.point_lights.size()),
                            static_cast<u32>(data.directional_lights.size())
                        };

                        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pbr.handle, context.dispatcher);
                        command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pbr.layout, 0, sets, nullptr, context.dispatcher);
                        command_buffer.pushConstants<u32>(pbr.layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, indices, context.dispatcher);
                    }

                    command_buffer.bindIndexBuffer(ibo.buffer.handle, 0, vk::IndexType::eUint32, context.dispatcher);
                    command_buffer.bindVertexBuffers(0, vbo.buffer.handle, static_cast<vk::DeviceSize>(0), context.dispatcher);
                    command_buffer.drawIndexed(index_count, 1, 0, 0, 0, context.dispatcher);
                }
            }
        }

        static void copy_to_swapchain() {
            auto& command_buffer = command_buffers[image_index];

            vk::ImageCopy copy{}; {
                copy.extent.width = context.swapchain.extent.width;
                copy.extent.height = context.swapchain.extent.height;
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
                copy_barrier.image = context.swapchain.images[image_index];
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
                context.dispatcher);

            command_buffer.copyImage(
                offscreen.color.handle, vk::ImageLayout::eTransferSrcOptimal,
                context.swapchain.images[image_index], vk::ImageLayout::eTransferDstOptimal,
                copy, context.dispatcher);

            vk::ImageMemoryBarrier present_barrier{}; {
                present_barrier.image = context.swapchain.images[image_index];
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
                context.dispatcher);
        }


        void draw(const RenderData& data) {
            image_index = context.device.logical.acquireNextImageKHR(context.swapchain.handle, -1, image_available[current_frame], nullptr, context.dispatcher).value;

            if (!in_flight[current_frame]) {
                vk::FenceCreateInfo fence_create_info{}; {
                    fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
                }

                in_flight[current_frame] = context.device.logical.createFence(fence_create_info, nullptr, context.dispatcher);
            }

            context.device.logical.waitForFences(in_flight[current_frame], true, -1, context.dispatcher);

            auto& command_buffer = command_buffers[image_index];

            vk::CommandBufferBeginInfo begin_info{}; {
                begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            }

            command_buffer.begin(begin_info, context.dispatcher);

            update_transforms(data);
            update_camera(data.camera);
            update_point_lights(data.point_lights);
            update_directional_lights(data.directional_lights);

            /* Final color pass */ {
                std::array<vk::ClearValue, 2> clear_values{}; {
                    clear_values[0].color = vk::ClearColorValue{ std::array{ 0.01f, 0.01f, 0.01f, 0.0f } };
                    clear_values[1].depthStencil = vk::ClearDepthStencilValue{ { 1.0f, 0 } };
                }

                vk::RenderPassBeginInfo render_pass_begin_info{}; {
                    render_pass_begin_info.renderArea.extent = context.swapchain.extent;
                    render_pass_begin_info.framebuffer = offscreen_framebuffer;
                    render_pass_begin_info.renderPass = offscreen_render_pass;
                    render_pass_begin_info.clearValueCount = clear_values.size();
                    render_pass_begin_info.pClearValues = clear_values.data();
                }

                vk::Viewport viewport{}; {
                    viewport.width = context.swapchain.extent.width;
                    viewport.height = -static_cast<float>(context.swapchain.extent.height);
                    viewport.x = 0;
                    viewport.y = context.swapchain.extent.height;
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                }

                vk::Rect2D scissor{}; {
                    scissor.extent = context.swapchain.extent;
                    scissor.offset = { { 0, 0 } };
                }

                command_buffer.setViewport(0, viewport, context.dispatcher);
                command_buffer.setScissor(0, scissor, context.dispatcher);

                command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline, context.dispatcher);
                final_draw_pass(data);
                command_buffer.endRenderPass(context.dispatcher);
            }

            copy_to_swapchain();

            command_buffer.end(context.dispatcher);
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

            context.device.logical.resetFences(in_flight[current_frame], context.dispatcher);
            context.device.queue.submit(submit_info, in_flight[current_frame], context.dispatcher);

            vk::PresentInfoKHR present_info{}; {
                present_info.waitSemaphoreCount = 1;
                present_info.pWaitSemaphores = &render_finished[current_frame];
                present_info.swapchainCount = 1;
                present_info.pSwapchains = &context.swapchain.handle;
                present_info.pImageIndices = &image_index;
            }

            context.device.queue.presentKHR(present_info, context.dispatcher);

            current_frame = (current_frame + 1) % api::frames_in_flight;
        }
    } // namespace tethys::renderer

    namespace texture {
        template <>
        Texture& get<texture::white>() {
            return renderer::builtin_textures[texture::white];
        }

        template <>
        Texture& get<texture::black>() {
            return renderer::builtin_textures[texture::black];
        }

        template <>
        Texture& get<texture::green>() {
            return renderer::builtin_textures[texture::green];
        }
    } // namespace tethys::texture

    namespace shader {
        template <>
        Pipeline& get<shader::minimal>() {
            return renderer::minimal;
        }

        template <>
        Pipeline& get<shader::generic>() {
            return renderer::generic;
        }

        template <>
        Pipeline& get<shader::pbr>() {
            return renderer::pbr;
        }
    } // namespace tethys::shader
} // namespace tethys