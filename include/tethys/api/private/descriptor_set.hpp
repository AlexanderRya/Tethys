#ifndef TETHYS_DESCRIPTOR_SET_HPP
#define TETHYS_DESCRIPTOR_SET_HPP

#include <tethys/api/private/single_descriptor_set.hpp>
#include <tethys/constants.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

#include <array>

namespace tethys::api {
    struct UpdateBufferInfo {
        std::array<vk::DescriptorBufferInfo, frames_in_flight> buffers;
        vk::DescriptorType type{};
        u64 binding{};
    };

    class DescriptorSet {
        std::array<SingleDescriptorSet, frames_in_flight> descriptor_sets;
    public:
        DescriptorSet() = default;

        void create(const vk::DescriptorSetLayout);
        void update(const UpdateBufferInfo&);
        void update(const std::vector<UpdateBufferInfo>&);
        void update(const UpdateImageInfo&);

        [[nodiscard]] SingleDescriptorSet& operator [](const usize);
        [[nodiscard]] const SingleDescriptorSet& operator [](const usize) const;
    };
} // namespace tethys::api

#endif //TETHYS_DESCRIPTOR_SET_HPP
