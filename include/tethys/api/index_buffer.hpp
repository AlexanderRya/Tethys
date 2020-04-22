#ifndef TETHYS_INDEX_BUFFER_HPP
#define TETHYS_INDEX_BUFFER_HPP

#include <tethys/api/static_buffer.hpp>
#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vector>

namespace tethys::api {
    struct IndexBuffer {
        StaticBuffer buffer{};
        usize size{};
    };

    [[nodiscard]] IndexBuffer make_index_buffer(std::vector<u32>&&);
} // namespace tethys::api

#endif //TETHYS_INDEX_BUFFER_HPP
