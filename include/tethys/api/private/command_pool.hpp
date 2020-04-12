#ifndef TETHYS_COMMANDPOOL_HPP
#define TETHYS_COMMANDPOOL_HPP

#include <tethys/Forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::CommandPool make_command_pool();
    [[nodiscard]] vk::CommandPool make_transient_pool();
} // namespace tethys::api

#endif //TETHYS_COMMANDPOOL_HPP
