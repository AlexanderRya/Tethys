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

    auto& generic = tethys::shader::get<tethys::shader::generic>();
    auto& minimal = tethys::shader::get<tethys::shader::minimal>();

    //auto nanosuit_model = tethys::renderer::upload_model("../resources/models/nanosuit/nanosuit.obj");
    auto dragon_model = tethys::renderer::upload_model("../resources/models/dragon/dragon.obj");
    auto suzanne_model = tethys::renderer::upload_model("../resources/models/suzanne/suzanne.obj");
    auto plane_model = tethys::renderer::upload_model("../resources/models/plane/plane.obj");
    auto sphere_mesh = tethys::renderer::upload_model(tethys::generate_sphere(1.0f, 36, 18), "../resources/textures/wood.png");
    //auto sponza_model = tethys::renderer::upload_model("../resources/models/sponza/sponza.obj");
    tethys::Model cube_mesh;

    /* Cube mesh */ {
        std::vector<float> cube_vertices_floats{
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
        };

        tethys::VertexData cube_data{}; {
            cube_data.geometry.resize(36);
            cube_data.indices.resize(36);
        }
        std::memcpy(cube_data.geometry.data(), cube_vertices_floats.data(), sizeof(float) * cube_vertices_floats.size());
        std::iota(cube_data.indices.begin(), cube_data.indices.end(), 0);

        cube_mesh = tethys::renderer::upload_model(cube_data, "../resources/textures/wood.png");
    }

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

    auto light_pos = glm::vec3(0.0f, 3.0f, 0.0f);

    glfwSetInputMode(tethys::window::handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float last_frame = 0;

    while (!tethys::window::should_close()) {
        tethys::RenderData data{};

        const float frame_time = glfwGetTime();
        float delta_time = frame_time - last_frame;
        last_frame = frame_time;

        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_RIGHT)) {
            light_pos.x += 1.25f * delta_time;
        }
        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_LEFT)) {
            light_pos.x -= 1.25f * delta_time;
        }
        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_RIGHT_SHIFT)) {
            light_pos.y += 1.25f * delta_time;
        }
        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_RIGHT_CONTROL)) {
            light_pos.y -= 1.25f * delta_time;
        }
        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_DOWN)) {
            light_pos.z += 1.25f * delta_time;
        }
        if (glfwGetKey(tethys::window::handle(), GLFW_KEY_UP)) {
            light_pos.z -= 1.25f * delta_time;
        }

        data.camera.projection = glm::perspective(glm::radians(60.f), 1280 / 720.0f, 0.1f, 1000.f);
        data.camera.view = camera.view();
        data.camera.pos = glm::vec4(camera.cam_pos, 0.0f);

        data.draw_commands = {
            tethys::DrawCommand{
                .model = dragon_model,
                .transform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, -0.5f)), glm::vec3(0.61f)),
                .shader = generic
            },
            tethys::DrawCommand{
                .model = suzanne_model,
                .transform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)), glm::vec3(0.45f)),
                .shader = generic
            },
            tethys::DrawCommand{
                .model = plane_model,
                .transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)),
                .shader = generic
            },
            /*tethys::DrawCommand{
                .model = sponza_model,
                .transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)),
                .shader = generic
            },*/
            tethys::DrawCommand{
                .model = sphere_mesh,
                .transform = glm::scale(glm::translate(glm::mat4(1.0f), light_pos), glm::vec3(0.15f)),
                .shader = minimal
            }
        };

        data.point_lights = {
            tethys::PointLight{
                .position = light_pos,
                .color = glm::vec3(1.0f),
                .falloff = {
                    .intensity = 1.0f,
                    .constant = 1.0f,
                    .linear = 0.14f,
                    .quadratic = 0.007f
                }
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