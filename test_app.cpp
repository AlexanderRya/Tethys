#include <tethys/renderer/renderer.hpp>
#include <tethys/window/window.hpp>
#include <tethys/constants.hpp>
#include <tethys/api/core.hpp>

#include <tethys/directional_light.hpp>
#include <tethys/render_data.hpp>
#include <tethys/point_light.hpp>
#include <tethys/handle.hpp>
#include <tethys/vertex.hpp>
#include <tethys/types.hpp>
#include <tethys/mesh.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include "camera_util.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

int main() {
    tethys::window::initialise(1280, 720, "Test");
    tethys::api::initialise();
    tethys::renderer::initialise();

    auto& minimal = tethys::shader::get<tethys::shader::minimal>();
    auto& generic = tethys::shader::get<tethys::shader::generic>();
    auto& pbr = tethys::shader::get<tethys::shader::pbr>();

    auto sphere_model = tethys::renderer::upload_model_pbr("../resources/models/sphere/sphere.obj");
    // Haccs
    sphere_model.submeshes[0].albedo = tethys::renderer::upload_texture("../resources/models/sphere/rustediron2_basecolor.png", vk::Format::eR8G8B8A8Srgb);
    sphere_model.submeshes[0].normal = tethys::renderer::upload_texture("../resources/models/sphere/rustediron2_normal.png", vk::Format::eR8G8B8A8Unorm);
    sphere_model.submeshes[0].roughness = tethys::renderer::upload_texture("../resources/models/sphere/rustediron2_roughness.png", vk::Format::eR8G8B8A8Unorm);
    sphere_model.submeshes[0].metallic = tethys::renderer::upload_texture("../resources/models/sphere/rustediron2_metallic.png", vk::Format::eR8G8B8A8Unorm);
    sphere_model.submeshes[0].occlusion.index = 0;

    auto sun = sphere_model;
    sun.submeshes[0].albedo.index = 0;
    sun.submeshes[0].normal.index = 0;
    sun.submeshes[0].roughness.index = 0;
    sun.submeshes[0].metallic.index = 0;
    sun.submeshes[0].occlusion.index = 0;

    static CameraUtil camera;

    glfwSetCursorPosCallback(tethys::window::handle(), [](GLFWwindow*, double xpos, double ypos) {
        static double lastX = 1280 / 2.0, lastY = 720 / 2.0;
        static bool first = true;

        if (first) {
            lastX = xpos;
            lastY = ypos;
            first = false;
        }

        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.process(xoffset, yoffset);
    });

    glfwSetInputMode(tethys::window::handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float last_frame = 0;

    while (!tethys::window::should_close()) {
        tethys::RenderData data{};

        const float frame_time = glfwGetTime();
        float delta_time = frame_time - last_frame;
        last_frame = frame_time;

        auto light_pos = glm::vec3(25.0f * std::sin(glfwGetTime()), 8.0f * std::sin(glfwGetTime()), 25.0f * std::cos(glfwGetTime()));

        data.camera = {
            glm::perspective(glm::radians(60.f), 1280 / 720.0f, 0.1f, 1000.f),
            camera.view(),
            glm::vec4(camera.cam_pos, 0.0f)
        };

        data.draw_commands = {
            tethys::DrawCommand{
                .model = sphere_model,
                .transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)),
                .shader = pbr
            },
            tethys::DrawCommand{
                .model = sun,
                .transform = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.10f)), light_pos),
                .shader = minimal
            }
        };

        data.point_lights = {
            tethys::PointLight{
                .position = light_pos,
                .color = glm::vec3(1.0f),
                .intensity = 6.0f,
                .constant = 1.0f,
                .linear = 0.14f,
                .quadratic = 0.007f
            }
        };

        tethys::renderer::draw(data);
        tethys::renderer::submit();

        camera.move(delta_time);

        if (tethys::window::key_pressed(GLFW_KEY_ESCAPE)) {
            tethys::window::close();
        }

        tethys::window::poll();
    }

    return 0;
}