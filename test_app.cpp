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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

#include "camera_util.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

int main() {
    tethys::window::initialise(1280, 720, "Test");
    tethys::api::initialise();
    tethys::renderer::initialise();

    auto nanosuit = tethys::renderer::upload_model("../resources/models/nanosuit/nanosuit.obj");

    std::vector<tethys::Vertex> plane_vertices{{
        { {  25.0f, -0.5f,  25.0f }, { 0.0f, 1.0f, 0.0f }, { 25.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { { -25.0f, -0.5f,  25.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f,  0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { { -25.0f, -0.5f, -25.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 25.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },

        { {  25.0f, -0.5f,  25.0f }, { 0.0f, 1.0f, 0.0f }, { 25.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { { -25.0f, -0.5f, -25.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 25.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { {  25.0f, -0.5f, -25.0f }, { 0.0f, 1.0f, 0.0f }, { 25.0f,25.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
    }};

    std::vector<unsigned> plane_indices(plane_vertices.size());
    std::iota(plane_indices.begin(), plane_indices.end(), 0);

    auto plane = tethys::renderer::upload_model(plane_vertices, plane_indices, "../resources/textures/wood.png");

    static CameraUtil camera;

    glfwSetCursorPosCallback(tethys::window::handle(), [](GLFWwindow*, const double xpos, const double ypos) {
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

        auto light_dir = glm::vec3(0.0f, 0.5f, -0.5f);
        auto projection = glm::perspective(glm::radians(60.f), 1280 / 720.f, 0.02f, 1000.f);

        data.camera.pv_mat = projection * camera.view();
        data.camera.pos = glm::vec4(camera.cam_pos, 0.0f);

        data.draw_commands = {
            tethys::DrawCommand{
                .model = nanosuit,
                .transform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)), glm::vec3(0.2f)),
                .shader = tethys::shader::generic
            },
            tethys::DrawCommand{
                .model = plane,
                .transform = glm::mat4(1.0f),
                .shader = tethys::shader::generic
            },
        };

        data.directional_lights = {
            tethys::DirectionalLight{
                .direction = light_dir,
                .color = glm::vec3(1.0f)
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