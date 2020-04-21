#include <tethys/experimental/rtx_renderer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/image.hpp>

namespace tethys::experimental::renderer {
    using namespace api;

    static struct {
        api::Image image;
        vk::ImageView view;
    } storage_image;

    void initialise() {
        api::Image::CreateInfo info{}; {
            info.tiling = vk::ImageTiling::eOptimal;
            info.width = ctx.swapchain.extent.width;
            info.height = ctx.swapchain.extent.height;
            info.format = vk::Format::eR8G8B8A8Unorm;
            info.usage_flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage;
            info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
        }

        storage_image.image = api::make_image(info);
        storage_image.view = api::make_image_view(storage_image.image.handle, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
        api::transition_image_layout(storage_image.image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
    }
} // namespace tethys::experimental::renderer