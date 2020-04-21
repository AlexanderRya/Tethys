#ifndef TETHYS_COMMAND_BUFFER_HPP
#define TETHYS_COMMAND_BUFFER_HPP

#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] std::vector<vk::CommandBuffer> make_rendering_command_buffers();
    [[nodiscard]] vk::CommandBuffer begin_transient();
    void end_transient(const vk::CommandBuffer);
} // namespace tethys::api

#endif //TETHYS_COMMAND_BUFFER_HPP
