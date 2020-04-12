#ifndef TETHYS_DESCRIPTORPOOL_HPP
#define TETHYS_DESCRIPTORPOOL_HPP

#include <tethys/Forwards.hpp>

namespace tethys::api {
    [[nodiscard]] vk::DescriptorPool make_descriptor_pool();
} // namespace tethys::api

#endif //TETHYS_DESCRIPTORPOOL_HPP
