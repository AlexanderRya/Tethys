#ifndef TETHYS_HANDLE_HPP
#define TETHYS_HANDLE_HPP

#include <tethys/Forwards.hpp>
#include <tethys/Types.hpp>

namespace tethys {
    template <typename Ty>
    struct Handle; // Empty

    template <>
    struct Handle<Mesh> {
        usize index;
    };
} // namespace tethys

#endif //TETHYS_HANDLE_HPP
