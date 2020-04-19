#include <tethys/api/private/descriptor_set.hpp>

namespace tethys::api {
    void DescriptorSet::create(const vk::DescriptorSetLayout layout) {
        for (auto& descriptor_set : descriptor_sets) {
            descriptor_set.create(layout);
        }
    }

    void DescriptorSet::update(const UpdateBufferInfo& info) {
        for (auto& descriptor_set : descriptor_sets) {
            for (const auto& buffer : info.buffers) {
                SingleUpdateBufferInfo single_info{}; {
                    single_info.buffer = buffer;
                    single_info.binding = info.binding;
                    single_info.type = info.type;
                }

                descriptor_set.update(single_info);
            }
        }
    }

    void DescriptorSet::update(const std::vector<UpdateBufferInfo>& infos) {
        for (auto& descriptor_set : descriptor_sets) {
            for (auto& info : infos) {
                for (const auto& buffer : info.buffers) {
                    SingleUpdateBufferInfo single_info{}; {
                        single_info.buffer = buffer;
                        single_info.binding = info.binding;
                        single_info.type = info.type;
                    }

                    descriptor_set.update(single_info);
                }
            }
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