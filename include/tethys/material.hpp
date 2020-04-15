#ifndef TETHYS_MATERIAL_HPP
#define TETHYS_MATERIAL_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>

namespace tethys {
    struct Material {
        Handle<Texture> texture;
        u32 shader;
    };
} // namespace tethys

#endif //TETHYS_MATERIAL_HPP
