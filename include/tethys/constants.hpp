#ifndef TETHYS_CONSTANTS_HPP
#define TETHYS_CONSTANTS_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

namespace tethys {
    namespace binding {
        // Set = 0
        constexpr inline u32 camera = 0;
        constexpr inline u32 transform = 1;
        constexpr inline u32 texture = 2;
        // Set = 1
        constexpr inline u32 point_light = 0;
        constexpr inline u32 directional_light = 1;
    } // namespace tethys::binding

    namespace layout {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
    } // namespace tethys::binding

    namespace shader {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
    } // namespace tethys::shader

    namespace texture {
        constexpr inline u32 white = 0;
    }

    namespace api {
        constexpr inline u32 frames_in_flight = 2;
    } // namespace tethys::api
} // namespace tethys

#endif //TETHYS_CONSTANTS_HPP

