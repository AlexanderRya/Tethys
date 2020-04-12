#ifndef TETHYS_CONSTANTS_HPP
#define TETHYS_CONSTANTS_HPP

#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

namespace tethys::api::meta {
    namespace binding {
        constexpr inline u32 camera = 0;
        constexpr inline u32 transform = 1;
        constexpr inline u32 material = 2;
        constexpr inline u32 texture = 3;
    } // namespace tethys::api::meta::binding

    constexpr inline u32 frames_in_flight = 2;
} // namespace tethys::api::meta

#endif //TETHYS_CONSTANTS_HPP

