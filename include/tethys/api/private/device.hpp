#ifndef TETHYS_DEVICE_HPP
#define TETHYS_DEVICE_HPP

#include <tethys/api/private/context.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    [[nodiscard]] Device make_device();
    [[nodiscard]] u64 find_memory_type(const u32, const vk::MemoryPropertyFlags&);
} // namespace tethys::api

#endif //TETHYS_DEVICE_HPP
