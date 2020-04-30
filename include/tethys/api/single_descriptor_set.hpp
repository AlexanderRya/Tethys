#ifndef TETHYS_SINGLE_DESCRIPTOR_SET_HPP
#define TETHYS_SINGLE_DESCRIPTOR_SET_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::api {
    struct SingleUpdateBufferInfo {
        vk::DescriptorBufferInfo buffer{};
        vk::DescriptorType type{};
        u64 binding{};
    };

    struct UpdateImageInfo {
        std::vector<vk::DescriptorImageInfo> image{};
        vk::DescriptorType type{};
        u64 binding{};
    };

    struct SingleUpdateImageInfo {
        vk::DescriptorImageInfo image{};
        vk::DescriptorType type{};
        u64 binding{};
    };

    class SingleDescriptorSet {
        vk::DescriptorSet descriptor_set{};
    public:
        void create(const vk::DescriptorSetLayout);
        void update(const SingleUpdateBufferInfo&);
        void update(const std::vector<SingleUpdateBufferInfo>&);
        void update(const UpdateImageInfo&);
        void update(const SingleUpdateImageInfo&);

        [[nodiscard]] vk::DescriptorSet handle() const;
    };
} // namespace tethys::api

#endif //TETHYS_SINGLE_DESCRIPTOR_SET_HPP
