#ifndef TETHYS_ACQUIRE_HPP
#define TETHYS_ACQUIRE_HPP

#include <tethys/types.hpp>

namespace tethys {
    // Defined for types:
    // tethys::Pipeline
    // tethys::PipelineLayout
    // vk::DescriptorSetLayout

    template <typename Ty>
    Ty& acquire(const u32);
} // namespace tethys

#endif //TETHYS_ACQUIRE_HPP
