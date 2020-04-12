#ifndef TETHYS_SWAPCHAIN_HPP
#define TETHYS_SWAPCHAIN_HPP

#include <tethys/api/private/Context.hpp>
#include <tethys/api/private/Image.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] Swapchain make_swapchain();
} // namespace tethys::api

#endif //TETHYS_SWAPCHAIN_HPP
