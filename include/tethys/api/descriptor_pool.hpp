#ifndef TETHYS_DESCRIPTOR_POOL_HPP
#define TETHYS_DESCRIPTOR_POOL_HPP

#include <tethys/forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::DescriptorPool make_descriptor_pool();
} // namespace tethys::api

#endif //TETHYS_DESCRIPTOR_POOL_HPP
