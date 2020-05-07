#include <tethys/api/instance.hpp>
#include <tethys/logger.hpp>
#include <tethys/util.hpp>

#include <GLFW/glfw3.h>

namespace tethys::api {
    [[maybe_unused]] VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void*) {
        std::printf("%s", util::format(
            "[{}] [Vulkan] [{}/{}]: {}\n",
            util::timestamp(),
            vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity)),
            vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(type)),
            data->pMessage).c_str());

        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::abort();
        }

        return 0;
    }

     std::vector<const char*> get_required_extensions() {
        u32 count = 0;

        auto required_extensions = glfwGetRequiredInstanceExtensions(&count);
        auto extensions = vk::enumerateInstanceExtensionProperties(nullptr, {}, context.dispatcher);

        std::vector<const char*> enabled_extensions;
        enabled_extensions.reserve(count + 2);

        for (u32 i = 0; i < count; ++i) {
            for (const auto& extension : extensions) {
                if (std::strcmp(extension.extensionName, required_extensions[i]) == 0) {
                    logger::info("Required extension activated: {}", enabled_extensions.emplace_back(required_extensions[i]));
                    break;
                }
            }
        }

        if (enabled_extensions.size() != count) {
            throw std::runtime_error("Required extension not supported.");
        }

#ifdef TETHYS_DEBUG
        enabled_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
         enabled_extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

         return enabled_extensions;
    }

    vk::Instance make_instance() {
        vk::ApplicationInfo application_info{}; {
            application_info.apiVersion = VK_API_VERSION_1_2;
            application_info.applicationVersion = VK_API_VERSION_1_2;
            application_info.engineVersion = VK_API_VERSION_1_2;
            application_info.pEngineName = "Tethys Rendering Engine";
            application_info.pApplicationName = "Tethys Rendering Engine";
        }

        auto enabled_exts = get_required_extensions();

        [[maybe_unused]] const char* validation_layer = "VK_LAYER_KHRONOS_validation";

        vk::InstanceCreateInfo instance_create_info{}; {
            instance_create_info.pApplicationInfo = &application_info;

            instance_create_info.ppEnabledExtensionNames = enabled_exts.data();
            instance_create_info.enabledExtensionCount = enabled_exts.size();
#ifdef TETHYS_DEBUG
            instance_create_info.ppEnabledLayerNames = &validation_layer;
            instance_create_info.enabledLayerCount = 1;
#else
            instance_create_info.ppEnabledLayerNames = nullptr;
            instance_create_info.enabledLayerCount = 0;
#endif
        }

        return vk::createInstance(instance_create_info, nullptr, context.dispatcher);
    }

    vk::DebugUtilsMessengerEXT install_validation_layers() {
        vk::DebugUtilsMessengerCreateInfoEXT create_info{}; {
            create_info.messageSeverity =
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            create_info.messageType =
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral    |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            create_info.pfnUserCallback = vulkan_debug_callback;
        }

        return context.instance.createDebugUtilsMessengerEXT(create_info, nullptr, context.dispatcher);
    }
} // namespace tethys::api