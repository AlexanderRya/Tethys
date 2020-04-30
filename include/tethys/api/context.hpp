#ifndef TETHYS_CONTEXT_HPP
#define TETHYS_CONTEXT_HPP

#include <tethys/api/image.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace tethys::api {
    struct Device {
        vk::PhysicalDevice physical{};
        vk::Device logical{};
        vk::Queue queue{};
        u32 queue_family{};
        vk::SampleCountFlagBits max_samples{};
    };

    struct Swapchain {
        vk::SwapchainKHR handle{};
        u32 image_count{};

        vk::Extent2D extent{};
        vk::SurfaceFormatKHR format{};
        vk::PresentModeKHR present_mode{};
        vk::SurfaceTransformFlagBitsKHR surface_transform{};

        std::vector<vk::Image> images{};
        std::vector<vk::ImageView> image_views{};
    };

    extern struct Context {
        vk::DispatchLoaderDynamic dispatcher{};
        VmaVulkanFunctions vma_dispatcher{};
        vk::Instance instance{};
        VmaAllocator allocator{};
        vk::DebugUtilsMessengerEXT validation{};
        vk::SurfaceKHR surface{};
        Device device{};
        Swapchain swapchain{};
        vk::CommandPool command_pool{};
        vk::CommandPool transient_pool{};
        vk::DescriptorPool descriptor_pool{};
        vk::Sampler default_sampler{};
    } ctx;
} // namespace tethys::api

#endif //TETHYS_CONTEXT_HPP
