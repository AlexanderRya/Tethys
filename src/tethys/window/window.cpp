#include <tethys/api/private/context.hpp>
#include <tethys/window/window.hpp>
#include <tethys/logger.hpp>
#include <tethys/util.hpp>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace tethys::window {
    static Window window;

    void initialise(const u32 width, const u32 height, const char* title) {
        glfwSetErrorCallback([](const i32 code, const char* message) {
            util::print(util::format(
                "[{}] [GLFW3] [Error: {}]: {}\n",
                util::timestamp(),
                code,
                message));
        });

        if (!glfwInit()) {
            throw std::runtime_error("Failed glfw init");
        }

        logger::info("glfwInit() success");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window.width = width;
        window.height = height;

        if (!(window.handle = glfwCreateWindow(width, height, title, nullptr, nullptr))) {
            throw std::runtime_error("Failed window creation");
        }

        logger::info("Window successfully created with size: ", width, "x", height);
    }

    bool should_close() {
        return glfwWindowShouldClose(window.handle);
    }

    vk::SurfaceKHR surface() {
        using namespace api;

        VkSurfaceKHR sf{};

        glfwCreateWindowSurface(static_cast<VkInstance>(ctx.instance), window.handle, nullptr, &sf);

        return sf;
    }

    u32 width() {
        return window.width;
    }

    u32 height() {
        return window.height;
    }

    const char* title() {
        return window.title;
    }

    void poll() {
        glfwPollEvents();
    }
} // namespace tethys::window