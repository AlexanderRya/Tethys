#include <tethys/renderer/renderer.hpp>
#include <tethys/window/window.hpp>
#include <tethys/api/core.hpp>

#include <tethys/directional_light.hpp>
#include <tethys/render_data.hpp>
#include <tethys/point_light.hpp>
#include <tethys/constants.hpp>
#include <tethys/model.hpp>

int main() {
    tethys::window::initialise(1280, 720, "Test");
    tethys::api::initialise();
    tethys::renderer::initialise();

    std::vector<tethys::Vertex> vertices{ {
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, {  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } },
        { {  0.0f,  0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f }, {  0.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }
    } };

    std::vector<tethys::u32> indices{
        0, 1, 2
    };

    auto triangle = tethys::renderer::upload_model(vertices, indices);

    while (!tethys::window::should_close()) {
        tethys::RenderData data{}; {
            data.draw_commands = {
                tethys::DrawCommand{
                    triangle,
                    glm::mat4(1.0f),
                    tethys::shader::minimal
                }
            };

            data.camera = {
                glm::mat4(1.0f),
                glm::vec4(0.0f),
            };
        }

        tethys::renderer::draw(data);
        tethys::renderer::submit();

        tethys::window::poll();
    }
    return 0;
}