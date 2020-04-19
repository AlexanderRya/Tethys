#ifndef TETHYS_MATERIAL_HPP
#define TETHYS_MATERIAL_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>

namespace tethys {
    struct Material {
        u32 shader;
        Handle<Texture> texture;
    };
} // namespace tethys

#endif //TETHYS_MATERIAL_HPP
