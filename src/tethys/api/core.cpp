#include <tethys/api/private/DescriptorPool.hpp>
#include <tethys/api/private/CommandPool.hpp>
#include <tethys/api/private/Framebuffer.hpp>
#include <tethys/api/private/RenderPass.hpp>
#include <tethys/api/private/Swapchain.hpp>
#include <tethys/api/private/Instance.hpp>
#include <tethys/api/private/Sampler.hpp>
#include <tethys/api/private/Context.hpp>
#include <tethys/api/private/Device.hpp>
#include <tethys/window/Window.hpp>
#include <tethys/Logger.hpp>
#include <tethys/Util.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tethys::api {
    static inline void load_vulkan_module() {
        auto module = util::load_module(util::vulkan_module);

        logger::info("Vulkan module: ", util::vulkan_module, " loaded at address: ", module);

        ctx.dispatcher.vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(util::load_symbol(module, "vkCreateInstance"));
        ctx.dispatcher.vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(util::load_symbol(module, "vkEnumerateInstanceExtensionProperties"));
        ctx.dispatcher.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(util::load_symbol(module, "vkGetInstanceProcAddr"));

        util::close_module(module);
    }

    static inline void load_vma() {
        logger::info("Initializing Vulkan Memory Allocator.");

        ctx.vma_dispatcher.vkAllocateMemory = ctx.dispatcher.vkAllocateMemory;
        ctx.vma_dispatcher.vkBindBufferMemory = ctx.dispatcher.vkBindBufferMemory;
        ctx.vma_dispatcher.vkBindBufferMemory2KHR = ctx.dispatcher.vkBindBufferMemory2KHR;
        ctx.vma_dispatcher.vkBindImageMemory = ctx.dispatcher.vkBindImageMemory;
        ctx.vma_dispatcher.vkBindImageMemory2KHR = ctx.dispatcher.vkBindImageMemory2KHR;
        ctx.vma_dispatcher.vkCmdCopyBuffer = ctx.dispatcher.vkCmdCopyBuffer;
        ctx.vma_dispatcher.vkCreateBuffer = ctx.dispatcher.vkCreateBuffer;
        ctx.vma_dispatcher.vkCreateImage = ctx.dispatcher.vkCreateImage;
        ctx.vma_dispatcher.vkDestroyBuffer = ctx.dispatcher.vkDestroyBuffer;
        ctx.vma_dispatcher.vkDestroyImage = ctx.dispatcher.vkDestroyImage;
        ctx.vma_dispatcher.vkFlushMappedMemoryRanges = ctx.dispatcher.vkFlushMappedMemoryRanges;
        ctx.vma_dispatcher.vkFreeMemory = ctx.dispatcher.vkFreeMemory;
        ctx.vma_dispatcher.vkGetBufferMemoryRequirements = ctx.dispatcher.vkGetBufferMemoryRequirements;
        ctx.vma_dispatcher.vkGetBufferMemoryRequirements2KHR = ctx.dispatcher.vkGetBufferMemoryRequirements2KHR;
        ctx.vma_dispatcher.vkGetImageMemoryRequirements = ctx.dispatcher.vkGetImageMemoryRequirements;
        ctx.vma_dispatcher.vkGetImageMemoryRequirements2KHR = ctx.dispatcher.vkGetImageMemoryRequirements2KHR;
        ctx.vma_dispatcher.vkGetPhysicalDeviceMemoryProperties = ctx.dispatcher.vkGetPhysicalDeviceMemoryProperties;
        ctx.vma_dispatcher.vkGetPhysicalDeviceMemoryProperties2KHR = ctx.dispatcher.vkGetPhysicalDeviceMemoryProperties2KHR;
        ctx.vma_dispatcher.vkGetPhysicalDeviceProperties = ctx.dispatcher.vkGetPhysicalDeviceProperties;
        ctx.vma_dispatcher.vkInvalidateMappedMemoryRanges = ctx.dispatcher.vkInvalidateMappedMemoryRanges;
        ctx.vma_dispatcher.vkMapMemory = ctx.dispatcher.vkMapMemory;
        ctx.vma_dispatcher.vkUnmapMemory = ctx.dispatcher.vkUnmapMemory;
    }

    [[nodiscard]] static inline VmaAllocator make_allocator() {
        VmaAllocatorCreateInfo allocator_create_info{}; {
            allocator_create_info.pVulkanFunctions = &ctx.vma_dispatcher;
            allocator_create_info.instance = static_cast<VkInstance>(ctx.instance);
            allocator_create_info.device = static_cast<VkDevice>(ctx.device.logical);
            allocator_create_info.physicalDevice = static_cast<VkPhysicalDevice>(ctx.device.physical);
            allocator_create_info.pHeapSizeLimit = nullptr;
            allocator_create_info.pRecordSettings = nullptr;
            allocator_create_info.pAllocationCallbacks = nullptr;
            allocator_create_info.pDeviceMemoryCallbacks = nullptr;
            allocator_create_info.frameInUseCount = 1;
            allocator_create_info.preferredLargeHeapBlockSize = 0;
            allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
        }

        VmaAllocator allocator{};

        if (vmaCreateAllocator(&allocator_create_info, &allocator) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create allocator");
        }

        logger::info("Vulkan Memory Allocator successfully initialized.");

        return allocator;
    }

    void initialise() {
        logger::info("Vulkan initialization sequence starting");
        load_vulkan_module();
        ctx.instance = make_instance();
        ctx.dispatcher.init(static_cast<VkInstance>(ctx.instance), ctx.dispatcher.vkGetInstanceProcAddr);
        load_vma();
#if defined(TETHYS_DEBUG)
        logger::warning("Vulkan debug mode active, performance may be lower than usual");
        ctx.validation = install_validation_layers();
#endif
        ctx.surface = tethys::window::surface();
        ctx.device = make_device();
        ctx.command_pool = make_command_pool();
        ctx.transient_pool = make_transient_pool();
        ctx.allocator = make_allocator();
        ctx.swapchain = make_swapchain();
        ctx.descriptor_pool = make_descriptor_pool();
        ctx.default_render_pass = make_default_render_pass();
        ctx.default_framebuffers = make_default_framebuffers();
        ctx.default_sampler = make_default_sampler();

        logger::info("Vulkan initialization sequence completed successfully");
    }
} // namespace tethys::api