#ifndef TETHYS_INSTANCE_HPP
#define TETHYS_INSTANCE_HPP

#include <tethys/api/private/context.hpp>

namespace tethys::api {
    [[nodiscard]] vk::Instance make_instance();
    [[nodiscard]] vk::DebugUtilsMessengerEXT install_validation_layers();
} // namespace tethys::api

#endif //TETHYS_INSTANCE_HPP
