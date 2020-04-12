#include <tethys/api/private/DescriptorSet.hpp>

namespace tethys::api {
    void DescriptorSet::create(const vk::DescriptorSetLayout layout) {
        for (auto& descriptor_set : descriptor_sets) {
            descriptor_set.create(layout);
        }
    }

    void DescriptorSet::update(const UpdateBufferInfo& info) {
        for (auto& descriptor_set : descriptor_sets) {
            descriptor_set.update(info);
        }
    }

    void DescriptorSet::update(const std::vector<UpdateBufferInfo>& info) {
        for (auto& descriptor_set : descriptor_sets) {
            descriptor_set.update(info);
        }
    }

    void DescriptorSet::update(const UpdateImageInfo& info) {
        for (auto& descriptor_set : descriptor_sets) {
            descriptor_set.update(info);
        }
    }

    SingleDescriptorSet& DescriptorSet::operator [](const usize idx) {
        return descriptor_sets[idx];
    }

    const SingleDescriptorSet& DescriptorSet::operator [](const usize idx) const {
        return descriptor_sets[idx];
    }
} // namespace tethys::api