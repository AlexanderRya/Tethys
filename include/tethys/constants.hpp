#ifndef TETHYS_CONSTANTS_HPP
#define TETHYS_CONSTANTS_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>

namespace tethys {
    namespace binding {
        // Set = 0
        constexpr inline u32 camera = 0;
        constexpr inline u32 transform = 1;
        constexpr inline u32 texture = 2;

        // Set = 1
        constexpr inline u32 shadow_map = 0;
        constexpr inline u32 point_light = 1;
        constexpr inline u32 directional_light = 2;
        constexpr inline u32 light_space = 3;
    } // namespace tethys::binding

    namespace layout {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
        constexpr inline u32 shadow = 2;
    } // namespace tethys::layout

    namespace shader {
        constexpr inline u32 generic = 0;
        constexpr inline u32 minimal = 1;
    } // namespace tethys::shader

    namespace texture {
        inline Handle<Texture> white{ 0 };
        inline Handle<Texture> black{ 1 };
        inline Handle<Texture> green{ 2 };
    } // namespace tethys::texture

    namespace api {
        constexpr inline u32 frames_in_flight = 2;
    } // namespace tethys::api
} // namespace tethys

#endif //TETHYS_CONSTANTS_HPP

