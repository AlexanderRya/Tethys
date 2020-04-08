#include <tethys/renderer/Renderer.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::api::renderer {
    struct RendererContext {
        std::vector<vk::Semaphore> image_available;
        std::vector<vk::Semaphore> render_finished;
        std::vector<vk::Fence> frames_in_flight;

        std::vector<vk::CommandBuffer> command_buffers;

        u32 image_index{};
        u32 current_frame{};
    } renderer_ctx;

    void initialize() {

    }
}