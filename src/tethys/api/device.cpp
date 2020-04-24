#include <tethys/api/context.hpp>
#include <tethys/api/device.hpp>

#include <tethys/logger.hpp>

namespace tethys::api {
    [[nodiscard]] static vk::SampleCountFlagBits get_max_sample_count(const vk::PhysicalDevice physical) {
        auto physical_device_properties = physical.getProperties(ctx.dispatcher);

        auto count =
            physical_device_properties.limits.framebufferColorSampleCounts &
            physical_device_properties.limits.framebufferDepthSampleCounts;

        if (count & vk::SampleCountFlagBits::e64) {
            return vk::SampleCountFlagBits::e64;
        }
        if (count & vk::SampleCountFlagBits::e32) {
            return vk::SampleCountFlagBits::e32;
        }
        if (count & vk::SampleCountFlagBits::e16) {
            return vk::SampleCountFlagBits::e16;
        }
        if (count & vk::SampleCountFlagBits::e8) {
            return vk::SampleCountFlagBits::e8;
        }
        if (count & vk::SampleCountFlagBits::e4) {
            return vk::SampleCountFlagBits::e4;
        }
        if (count & vk::SampleCountFlagBits::e2) {
            return vk::SampleCountFlagBits::e2;
        }

        return vk::SampleCountFlagBits::e1;
    }

     [[nodiscard]] static vk::PhysicalDevice get_physical_device() {
        auto physical_devices = ctx.instance.enumeratePhysicalDevices(ctx.dispatcher);

        for (const auto& device : physical_devices) {
            auto device_properties = device.getProperties(ctx.dispatcher);
            auto device_features = device.getFeatures(ctx.dispatcher);

            if ((device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu  ||
                 device_properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ||
                 device_properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu) &&

                device_features.shaderSampledImageArrayDynamicIndexing &&
                device_features.samplerAnisotropy &&
                device_features.multiDrawIndirect) {

                auto major = device_properties.apiVersion >> 22u;
                auto minor = device_properties.apiVersion >> 12u & 0x3ffu;
                auto patch = device_properties.apiVersion & 0xfffu;

                logger::info("Selected physical device: ", device_properties.deviceName);
                logger::info("Vulkan version: ", major, ".", minor, ".", patch);
                return device;
            }
        }

        throw std::runtime_error("No suitable physical device available");
    }

    [[nodiscard]] static u32 get_queue_family(const vk::SurfaceKHR& surface, const vk::PhysicalDevice& physical_device, const vk::DispatchLoaderDynamic& dispatcher) {
        auto queue_family_properties = physical_device.getQueueFamilyProperties({}, dispatcher);

        for (u32 i = 0; i < queue_family_properties.size(); ++i) {
            if (((queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics) &&
                physical_device.getSurfaceSupportKHR(i, surface, dispatcher)) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find a queue family");
    }

    [[nodiscard]] static vk::Device get_device(const u32 queue_family, const vk::PhysicalDevice& physical_device, const vk::DispatchLoaderDynamic& dispatcher) {
        auto extensions = physical_device.enumerateDeviceExtensionProperties(nullptr, {}, dispatcher);

        constexpr std::array enabled_exts{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            // Not supported by renderdoc rip
            /*VK_KHR_RAY_TRACING_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME*/
        };

        if (std::find_if(extensions.begin(), extensions.end(), [&enabled_exts](const vk::ExtensionProperties& properties) {
            return std::any_of(enabled_exts.begin(), enabled_exts.end(), [&properties](const char* name) {
                return std::strcmp(name, properties.extensionName);
            });
        }) != extensions.end()) {
            float priorities[]{ 1.0f };

            vk::DeviceQueueCreateInfo queue_create_info{}; {
                queue_create_info.queueCount = 1;
                queue_create_info.queueFamilyIndex = queue_family;
                queue_create_info.pQueuePriorities = priorities;
            }

            vk::PhysicalDeviceFeatures features{}; {
                features.samplerAnisotropy = true;
                features.multiDrawIndirect = true;
                features.sampleRateShading = true;
            }

            vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{}; {
                descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = true;
                descriptor_indexing_features.descriptorBindingVariableDescriptorCount = true;
                descriptor_indexing_features.descriptorBindingPartiallyBound = true;
                descriptor_indexing_features.runtimeDescriptorArray = true;
            }

            vk::PhysicalDeviceRayTracingFeaturesKHR ray_tracing_features{}; {
                ray_tracing_features.pNext = &descriptor_indexing_features;
                ray_tracing_features.rayTracing = true;
            }

            vk::DeviceCreateInfo device_create_info{}; {
                device_create_info.pNext = &ray_tracing_features;
                device_create_info.ppEnabledExtensionNames = enabled_exts.data();
                device_create_info.enabledExtensionCount = enabled_exts.size();
                device_create_info.pQueueCreateInfos = &queue_create_info;
                device_create_info.queueCreateInfoCount = 1;
                device_create_info.pEnabledFeatures = &features;
            }

            return physical_device.createDevice(device_create_info, nullptr, dispatcher);
        } else {
            throw std::runtime_error("Selected physical device does not support a swapchain");
        }
    }

     vk::Queue get_queue(const vk::Device& device, const u32 queue_family, const vk::DispatchLoaderDynamic& dispatcher) {
        return device.getQueue(queue_family, 0, dispatcher);
    }

    Device make_device() {
        Device device{};

        device.physical = get_physical_device();
        device.queue_family = get_queue_family(ctx.surface, device.physical, ctx.dispatcher);
        device.logical = get_device(device.queue_family, device.physical, ctx.dispatcher);
        device.queue = get_queue(device.logical, device.queue_family, ctx.dispatcher);
        device.max_samples = get_max_sample_count(device.physical);

        return device;
    }

    u64 find_memory_type(const u32 mask, const vk::MemoryPropertyFlags& flags) {
        const vk::PhysicalDeviceMemoryProperties memory_properties = ctx.device.physical.getMemoryProperties(ctx.dispatcher);

        for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
            if ((mask & (1u << i)) &&
                (memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find memory type");
    }
} // namespace tethys::api