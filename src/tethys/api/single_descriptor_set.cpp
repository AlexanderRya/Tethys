#include <tethys/api/single_descriptor_set.hpp>
#include <tethys/api/context.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    void SingleDescriptorSet::create(const vk::DescriptorSetLayout layout) {
        vk::DescriptorSetAllocateInfo info{}; {
            info.descriptorSetCount = 1;
            info.descriptorPool = context.descriptor_pool;
            info.pSetLayouts = &layout;
        }

        descriptor_set = context.device.logical.allocateDescriptorSets(info, context.dispatcher).back();
    }

    void SingleDescriptorSet::update(const SingleUpdateBufferInfo& info) {
        vk::WriteDescriptorSet write{}; {
            write.descriptorCount = 1;
            write.pImageInfo = nullptr;
            write.pTexelBufferView = nullptr;
            write.pBufferInfo = &info.buffer;
            write.dstSet = descriptor_set;
            write.dstBinding = info.binding;
            write.dstArrayElement = 0;
            write.descriptorType = info.type;
        }

        context.device.logical.updateDescriptorSets(write, nullptr, context.dispatcher);
    }

    void SingleDescriptorSet::update(const std::vector<SingleUpdateBufferInfo>& info) {
        for (const auto& each : info) {
            vk::WriteDescriptorSet write{}; {
                write.descriptorCount = 1;
                write.pImageInfo = nullptr;
                write.pTexelBufferView = nullptr;
                write.pBufferInfo = &each.buffer;
                write.dstSet = descriptor_set;
                write.dstBinding = each.binding;
                write.dstArrayElement = 0;
                write.descriptorType = each.type;
            }

            context.device.logical.updateDescriptorSets(write, nullptr, context.dispatcher);
        }
    }

    void SingleDescriptorSet::update(const UpdateImageInfo& info) {
        vk::WriteDescriptorSet write{}; {
            write.descriptorCount = info.size;
            write.pImageInfo = info.image;
            write.pTexelBufferView = nullptr;
            write.pBufferInfo = nullptr;
            write.dstSet = descriptor_set;
            write.dstBinding = info.binding;
            write.dstArrayElement = 0;
            write.descriptorType = info.type;
        }

        context.device.logical.updateDescriptorSets(write, nullptr, context.dispatcher);
    }

    void SingleDescriptorSet::update(const SingleUpdateImageInfo& info) {
        vk::WriteDescriptorSet write{}; {
            write.descriptorCount = 1;
            write.pImageInfo = &info.image;
            write.pTexelBufferView = nullptr;
            write.pBufferInfo = nullptr;
            write.dstSet = descriptor_set;
            write.dstBinding = info.binding;
            write.dstArrayElement = 0;
            write.descriptorType = info.type;
        }

        context.device.logical.updateDescriptorSets(write, nullptr, context.dispatcher);
    }

    vk::DescriptorSet SingleDescriptorSet::handle() const {
        return descriptor_set;
    }
} // namespace tethys::api