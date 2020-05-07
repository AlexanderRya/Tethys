#include <tethys/api/command_pool.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::CommandPool make_command_pool() {
        vk::CommandPoolCreateInfo command_pool_create_info{}; {
            command_pool_create_info.queueFamilyIndex = context.device.family;
            command_pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        }

        auto pool = context.device.logical.createCommandPool(command_pool_create_info, nullptr, context.dispatcher);

        logger::info("Command pool successfully created");

        return pool;
    }

    vk::CommandPool make_transient_pool() {
        vk::CommandPoolCreateInfo command_pool_create_info{}; {
            command_pool_create_info.queueFamilyIndex = context.device.family;
            command_pool_create_info.flags =
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
                vk::CommandPoolCreateFlagBits::eTransient;
        }

        auto pool = context.device.logical.createCommandPool(command_pool_create_info, nullptr, context.dispatcher);

        logger::info("Transient command pool successfully created");

        return pool;
    }
} // namespace tethys::api