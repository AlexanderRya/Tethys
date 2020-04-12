#ifndef TETHYS_WINDOW_HPP
#define TETHYS_WINDOW_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

namespace tethys::window {
    struct Window {
        u32 width;
        u32 height;
        const char* title;
        GLFWwindow* handle;
    };

    void initialise(const u32, const u32, const char*);

    [[nodiscard]] bool should_close();
    [[nodiscard]] vk::SurfaceKHR surface();
    [[nodiscard]] u32 width();
    [[nodiscard]] u32 height();
    [[nodiscard]] const char* title();
    void poll();
} // namespace tethys::window

#endif //TETHYS_WINDOW_HPP
