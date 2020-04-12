#ifndef TETHYS_COMMAND_POOL_HPP
#define TETHYS_COMMAND_POOL_HPP

#include <tethys/forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::CommandPool make_command_pool();
    [[nodiscard]] vk::CommandPool make_transient_pool();
} // namespace tethys::api

#endif //TETHYS_COMMAND_POOL_HPP
