#ifndef TETHYS_CONSTANTS_HPP
#define TETHYS_CONSTANTS_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

namespace tethys::api::meta {
    namespace binding {
        constexpr inline u32 camera = 0;
        constexpr inline u32 transform = 1;
        constexpr inline u32 point_light = 2;
        constexpr inline u32 texture = 3;
    } // namespace tethys::api::meta::binding

    namespace layout {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
    } // namespace layout

    namespace shader {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
    } // namespace tethys::api::meta::shader

    constexpr inline u32 frames_in_flight = 2;
} // namespace tethys::api::meta

#endif //TETHYS_CONSTANTS_HPP

