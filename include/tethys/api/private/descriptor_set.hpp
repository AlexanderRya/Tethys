#ifndef TETHYS_DESCRIPTORSET_HPP
#define TETHYS_DESCRIPTORSET_HPP

#include <tethys/api/private/SingleDescriptorSet.hpp>
#include <tethys/api/meta/constants.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

#include <array>

namespace tethys::api {
    class DescriptorSet {
        std::array<SingleDescriptorSet, meta::frames_in_flight> descriptor_sets;
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

#endif //TETHYS_DESCRIPTORSET_HPP
