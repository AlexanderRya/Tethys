#ifndef TETHYS_COMMANDBUFFER_HPP
#define TETHYS_COMMANDBUFFER_HPP

#include <tethys/Forwards.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] std::vector<vk::CommandBuffer> make_rendering_command_buffers();
    [[nodiscard]] vk::CommandBuffer begin_transient();
    void end_transient(const vk::CommandBuffer);
} // namespace tethys::api

#endif //TETHYS_COMMANDBUFFER_HPP
