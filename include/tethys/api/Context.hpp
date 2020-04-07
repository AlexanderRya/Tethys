#ifndef TETHYS_CONTEXT_HPP
#define TETHYS_CONTEXT_HPP

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    struct VulkanContext {
        vk::DispatchLoaderDynamic dispatcher;
        vk::Instance instance;
    };

    [[nodiscard]] VulkanContext& context();
} // namespace tethys::api

#endif //TETHYS_CONTEXT_HPP
