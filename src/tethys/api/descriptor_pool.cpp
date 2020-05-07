#include <tethys/api/descriptor_pool.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::DescriptorPool make_descriptor_pool() {
        std::array<vk::DescriptorPoolSize, 3> descriptor_pool_sizes{ {
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eUniformBuffer, 100000 },
            { vk::DescriptorType::eStorageBuffer, 100000 },
        } };

        vk::DescriptorPoolCreateInfo descriptor_pool_create_info{}; {
            descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
            descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
            descriptor_pool_create_info.maxSets = descriptor_pool_sizes.size() * 100000;
        }

        auto pool = context.device.logical.createDescriptorPool(descriptor_pool_create_info, nullptr, context.dispatcher);

        logger::info("Descriptor pool successfully created, sizes:");
        for (const auto& [type, count] : descriptor_pool_sizes) {
            logger::info("Type: {}, Count: {}", vk::to_string(type), count);
        }

        return pool;
    }
} // namespace tethys::api