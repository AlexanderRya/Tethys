#ifndef TETHYS_HANDLE_HPP
#define TETHYS_HANDLE_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

namespace tethys {
    template <typename Ty>
    struct Handle {
        usize index;
    };
} // namespace tethys

#endif //TETHYS_HANDLE_HPP
