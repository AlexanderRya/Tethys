#include <tethys/api/Context.hpp>

namespace tethys::api {
    static VulkanContext ctx;

    VulkanContext& context() {
        return ctx;
    }
} // namespace tethys::api