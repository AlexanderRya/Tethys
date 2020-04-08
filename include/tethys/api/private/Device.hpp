#ifndef TETHYS_DEVICE_HPP
#define TETHYS_DEVICE_HPP

#include <tethys/api/private/InternalContext.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    [[nodiscard]] Device make_device();
    [[nodiscard]] u64 find_memory_type(const u32, const vk::MemoryPropertyFlags&);
} // namespace tethys::api

#endif //TETHYS_DEVICE_HPP
