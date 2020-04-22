#ifndef TETHYS_HANDLE_HPP
#define TETHYS_HANDLE_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

namespace tethys {
    template <typename Ty>
    struct Handle {
        usize index{};
    };

    template <>
    struct Handle<Mesh> {
        usize vbo_index{};
        usize ibo_index{};
    };
} // namespace tethys

#endif //TETHYS_HANDLE_HPP
