#include <tethys/api/descriptor_pool.hpp>
#include <tethys/api/command_pool.hpp>
#include <tethys/api/framebuffer.hpp>
#include <tethys/api/render_pass.hpp>
#include <tethys/api/swapchain.hpp>
#include <tethys/window/window.hpp>
#include <tethys/api/instance.hpp>
#include <tethys/api/sampler.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/device.hpp>
#include <tethys/logger.hpp>
#include <tethys/util.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tethys::api {
     static void load_vulkan_module() {
        auto module = util::load_module(util::vulkan_module);

        logger::info("Vulkan module: ", util::vulkan_module, " loaded at address: ", module);

         context.dispatcher.vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(util::load_symbol(module, "vkCreateInstance"));
         context.dispatcher.vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(util::load_symbol(module, "vkEnumerateInstanceExtensionProperties"));
         context.dispatcher.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(util::load_symbol(module, "vkGetInstanceProcAddr"));

        util::close_module(module);
    }

     static void load_vma() {
        logger::info("Initializing Vulkan Memory Allocator.");

         context.vma_dispatcher.vkAllocateMemory = context.dispatcher.vkAllocateMemory;
         context.vma_dispatcher.vkBindBufferMemory = context.dispatcher.vkBindBufferMemory;
         context.vma_dispatcher.vkBindBufferMemory2KHR = context.dispatcher.vkBindBufferMemory2KHR;
         context.vma_dispatcher.vkBindImageMemory = context.dispatcher.vkBindImageMemory;
         context.vma_dispatcher.vkBindImageMemory2KHR = context.dispatcher.vkBindImageMemory2KHR;
         context.vma_dispatcher.vkCmdCopyBuffer = context.dispatcher.vkCmdCopyBuffer;
         context.vma_dispatcher.vkCreateBuffer = context.dispatcher.vkCreateBuffer;
         context.vma_dispatcher.vkCreateImage = context.dispatcher.vkCreateImage;
         context.vma_dispatcher.vkDestroyBuffer = context.dispatcher.vkDestroyBuffer;
         context.vma_dispatcher.vkDestroyImage = context.dispatcher.vkDestroyImage;
         context.vma_dispatcher.vkFlushMappedMemoryRanges = context.dispatcher.vkFlushMappedMemoryRanges;
         context.vma_dispatcher.vkFreeMemory = context.dispatcher.vkFreeMemory;
         context.vma_dispatcher.vkGetBufferMemoryRequirements = context.dispatcher.vkGetBufferMemoryRequirements;
         context.vma_dispatcher.vkGetBufferMemoryRequirements2KHR = context.dispatcher.vkGetBufferMemoryRequirements2KHR;
         context.vma_dispatcher.vkGetImageMemoryRequirements = context.dispatcher.vkGetImageMemoryRequirements;
         context.vma_dispatcher.vkGetImageMemoryRequirements2KHR = context.dispatcher.vkGetImageMemoryRequirements2KHR;
         context.vma_dispatcher.vkGetPhysicalDeviceMemoryProperties = context.dispatcher.vkGetPhysicalDeviceMemoryProperties;
         context.vma_dispatcher.vkGetPhysicalDeviceMemoryProperties2KHR = context.dispatcher.vkGetPhysicalDeviceMemoryProperties2KHR;
         context.vma_dispatcher.vkGetPhysicalDeviceProperties = context.dispatcher.vkGetPhysicalDeviceProperties;
         context.vma_dispatcher.vkInvalidateMappedMemoryRanges = context.dispatcher.vkInvalidateMappedMemoryRanges;
         context.vma_dispatcher.vkMapMemory = context.dispatcher.vkMapMemory;
         context.vma_dispatcher.vkUnmapMemory = context.dispatcher.vkUnmapMemory;
    }

     [[nodiscard]] static VmaAllocator make_allocator() {
        VmaAllocatorCreateInfo allocator_create_info{}; {
            allocator_create_info.pVulkanFunctions = &context.vma_dispatcher;
            allocator_create_info.instance = static_cast<VkInstance>(context.instance);
            allocator_create_info.device = static_cast<VkDevice>(context.device.logical);
            allocator_create_info.physicalDevice = static_cast<VkPhysicalDevice>(context.device.physical);
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
        context.instance = make_instance();
        context.dispatcher.init(static_cast<VkInstance>(context.instance), context.dispatcher.vkGetInstanceProcAddr);
        load_vma();
#if defined(TETHYS_DEBUG)
        logger::warning("Vulkan debug mode active, performance may be lower than usual");
        context.validation = install_validation_layers();
#endif
        context.surface = tethys::window::surface();
        context.device = make_device();
        context.command_pool = make_command_pool();
        context.transient_pool = make_transient_pool();
        context.allocator = make_allocator();
        context.swapchain = make_swapchain();
        context.descriptor_pool = make_descriptor_pool();
        make_samplers();

        logger::info("Vulkan initialization sequence completed successfully");
    }
} // namespace tethys::api