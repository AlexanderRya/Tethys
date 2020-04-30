#ifndef TETHYS_SWAPCHAIN_HPP
#define TETHYS_SWAPCHAIN_HPP

#include <tethys/api/context.hpp>
#include <tethys/api/image.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] Swapchain make_swapchain();
} // namespace tethys::api

#endif //TETHYS_SWAPCHAIN_HPP
