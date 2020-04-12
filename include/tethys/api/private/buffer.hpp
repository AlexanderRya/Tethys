#ifndef TETHYS_BUFFER_HPP
#define TETHYS_BUFFER_HPP

#include <tethys/api/private/SingleBuffer.hpp>
#include <tethys/api/meta/constants.hpp>

namespace tethys::api {
    template <typename Ty>
    class Buffer {
        std::array<SingleBuffer<Ty>, meta::frames_in_flight> buffers;
    public:
        Buffer() = default;
        void create(const vk::BufferUsageFlags);
        void allocate(const usize);
        void write(const Ty&);
        void write(const std::vector<Ty>&);
        void deallocate();

        [[nodiscard]] std::array<vk::DescriptorBufferInfo, meta::frames_in_flight> info() const;
        [[nodiscard]] SingleBuffer<Ty>& operator [](const usize);
        [[nodiscard]] const SingleBuffer<Ty>& operator [](const usize) const;
    };

    template <typename Ty>
    void Buffer<Ty>::create(const vk::BufferUsageFlags flags) {
        for (auto& buffer : buffers) {
            buffer.create(flags);
        }
    }

    template <typename Ty>
    void Buffer<Ty>::allocate(const usize size) {
        for (auto& buffer : buffers) {
            buffer.allocate(size);
        }
    }

    template <typename Ty>
    void Buffer<Ty>::write(const Ty& obj) {
        for (auto& buffer : buffers) {
            buffer.write(obj);
        }
    }

    template <typename Ty>
    void Buffer<Ty>::write(const std::vector<Ty>& objs) {
        for (auto& buffer : buffers) {
            buffer.write(objs);
        }
    }

    template <typename Ty>
    void Buffer<Ty>::deallocate() {
        for (auto& buffer : buffers) {
            buffer.deallocate();
        }
    }

    template <typename Ty>
    std::array<vk::DescriptorBufferInfo, meta::frames_in_flight> Buffer<Ty>::info() const {
        std::array<vk::DescriptorBufferInfo, meta::frames_in_flight> infos{};

        for (int i = 0; i < meta::frames_in_flight; ++i) {
            infos[i] = buffers[i].info();
        }

        return infos;
    }

    template <typename Ty>
    SingleBuffer<Ty>& Buffer<Ty>::operator [](const usize idx) {
        return buffers[idx];
    }

    template <typename Ty>
    const SingleBuffer<Ty>& Buffer<Ty>::operator [](const usize idx) const {
        return buffers[idx];
    }
} // namespace tethys::api

#endif //TETHYS_BUFFER_HPP
