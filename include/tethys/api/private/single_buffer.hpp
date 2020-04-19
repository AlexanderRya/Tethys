#ifndef TETHYS_SINGLE_BUFFER_HPP
#define TETHYS_SINGLE_BUFFER_HPP

#include <tethys/api/private/static_buffer.hpp>
#include <tethys/api/private/context.hpp>

#include <type_traits>

namespace tethys::api {
    template <typename Ty>
    class SingleBuffer {
        usize current_capacity{};
        usize current_size{};
        StaticBuffer buffer{};
        void* mapped{};

    public:
        SingleBuffer() = default;
        void create(const vk::BufferUsageFlags);
        void allocate(const usize);
        void write(const Ty&);
        void write(const std::vector<Ty>&);
        void deallocate();

        [[nodiscard]] usize size() const;
        [[nodiscard]] vk::DescriptorBufferInfo info() const;
    };

    template <typename Ty>
    void SingleBuffer<Ty>::create(const vk::BufferUsageFlags flags) {
        current_capacity = 16;
        current_size = 0;
        buffer = make_buffer(current_capacity * sizeof(Ty), flags, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT);
        vmaMapMemory(ctx.allocator, buffer.allocation, &mapped);
    }

    template <typename Ty>
    void SingleBuffer<Ty>::allocate(const usize capacity) {
        buffer = make_buffer(capacity * sizeof(Ty), buffer.flags, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT);
        vmaMapMemory(ctx.allocator, buffer.allocation, &mapped);
        current_capacity = capacity;
    }

    template <typename Ty>
    void SingleBuffer<Ty>::write(const Ty& obj) {
        static_assert(std::is_trivially_copyable_v<Ty>, "Type is not trivially copyable!");
        current_size = 1;

        std::memcpy(mapped, &obj, sizeof(Ty));
    }

    template <typename Ty>
    void SingleBuffer<Ty>::write(const std::vector<Ty>& objs) {
        static_assert(std::is_trivially_copyable_v<Ty>, "Type is not trivially copyable!");
        if (objs.size() > current_capacity) {
            deallocate();
            allocate(objs.capacity());
        }

        std::memcpy(mapped, objs.data(), objs.size() * sizeof(Ty));
        current_size = objs.size();
    }

    template <typename Ty>
    void SingleBuffer<Ty>::deallocate() {
        vmaUnmapMemory(ctx.allocator, buffer.allocation);
        destroy_buffer(buffer);
    }

    template <typename Ty>
    usize SingleBuffer<Ty>::size() const {
        return current_size;
    }

    template <typename Ty>
    vk::DescriptorBufferInfo SingleBuffer<Ty>::info() const {
        vk::DescriptorBufferInfo buffer_info{}; {
            buffer_info.buffer = buffer.handle;
            buffer_info.range = current_size ? current_size * sizeof(Ty) : VK_WHOLE_SIZE;
            buffer_info.offset = 0ull;
        }

        return buffer_info;
    }
} // namespace tethys::api

#endif //TETHYS_SINGLE_BUFFER_HPP
