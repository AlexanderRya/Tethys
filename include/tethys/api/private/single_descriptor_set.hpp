#ifndef TETHYS_SINGLEDESCRIPTORSET_HPP
#define TETHYS_SINGLEDESCRIPTORSET_HPP

#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::api {
    struct UpdateBufferInfo {
        vk::DescriptorBufferInfo buffer{};
        vk::DescriptorType type{};
        u64 binding{};
    };

    struct UpdateImageInfo {
        std::vector<vk::DescriptorImageInfo> image{};
        vk::DescriptorType type{};
        u64 binding{};
    };

    class SingleDescriptorSet {
        vk::DescriptorSet descriptor_set{};
    public:
        void create(const vk::DescriptorSetLayout);
        void update(const UpdateBufferInfo&);
        void update(const std::vector<UpdateBufferInfo>&);
        void update(const UpdateImageInfo&);

        [[nodiscard]] vk::DescriptorSet handle() const;
    };
} // namespace tethys::api

#endif //TETHYS_SINGLEDESCRIPTORSET_HPP
