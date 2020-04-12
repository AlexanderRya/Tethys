#include <tethys/api/private/single_descriptor_set.hpp>
#include <tethys/api/private/context.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    void SingleDescriptorSet::create(const vk::DescriptorSetLayout layout) {
        vk::DescriptorSetAllocateInfo info{}; {
            info.descriptorSetCount = 1;
            info.descriptorPool = ctx.descriptor_pool;
            info.pSetLayouts = &layout;
        }

        descriptor_set = ctx.device.logical.allocateDescriptorSets(info, ctx.dispatcher).back();
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

        ctx.device.logical.updateDescriptorSets(write, nullptr, ctx.dispatcher);
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

            ctx.device.logical.updateDescriptorSets(write, nullptr, ctx.dispatcher);
        }
    }

    void SingleDescriptorSet::update(const UpdateImageInfo& info) {
        vk::WriteDescriptorSet write{}; {
            write.descriptorCount = info.image.size();
            write.pImageInfo = info.image.data();
            write.pTexelBufferView = nullptr;
            write.pBufferInfo = nullptr;
            write.dstSet = descriptor_set;
            write.dstBinding = info.binding;
            write.dstArrayElement = 0;
            write.descriptorType = info.type;
        }

        ctx.device.logical.updateDescriptorSets(write, nullptr, ctx.dispatcher);
    }

    vk::DescriptorSet SingleDescriptorSet::handle() const {
        return descriptor_set;
    }
} // namespace tethys::api