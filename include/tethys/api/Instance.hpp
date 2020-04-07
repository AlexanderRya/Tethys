#ifndef TETHYS_INSTANCE_HPP
#define TETHYS_INSTANCE_HPP

#include <tethys/api/Context.hpp>

namespace tethys::api {
    [[nodiscard]] vk::Instance make_instance(const VulkanContext&);
    [[nodiscard]] vk::DebugUtilsMessengerEXT install_validation_layers(const VulkanContext&);
} // namespace tethys::api

#endif //TETHYS_INSTANCE_HPP
