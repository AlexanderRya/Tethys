#include <tethys/mesh.hpp>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <cmath>

namespace tethys {
    VertexData generate_sphere(const f32 radius, const u32 sectors, const u32 stacks) {
        std::vector<Vertex> geometry{};
        std::vector<u32> indices{};

        constexpr f64 pi = 3.141592653589793238462643383279502884197169399375;

        f32 length_inverse = 1.0f / radius;
        f32 sector_step = 2 * pi / sectors;
        f32 stack_step = pi / stacks;

        for (u32 i = 0; i <= stacks; ++i) {
            f32 stack_angle = pi / 2 - i * stack_step;
            for (u32 j = 0; j <= sectors; ++j) {
                Vertex vertex{};
                f32 sector_angle = j * sector_step;
                vertex.pos = {
                    radius * std::cos(stack_angle) * std::cos(sector_angle),
                    radius * std::cos(stack_angle) * std::sin(sector_angle),
                    radius * std::sin(stack_angle)
                };
                vertex.norms = {
                    vertex.pos.x * length_inverse,
                    vertex.pos.y * length_inverse,
                    vertex.pos.z * length_inverse
                };
                vertex.uvs = {
                    j / static_cast<f32>(sectors),
                    i / static_cast<f32>(stacks)
                };

                geometry.emplace_back(vertex);
            }
        }

        for (u32 i = 0; i < stacks; ++i) {
            i32 k1 = i * (sectors + 1);
            i32 k2 = k1 + sectors + 1;

            for (u32 j = 0; j < sectors; ++j, ++k1, ++k2) {
                if (i != 0) {
                    indices.emplace_back(k1);
                    indices.emplace_back(k2);
                    indices.emplace_back(k1 + 1);
                }

                if (i != (stacks - 1)) {
                    indices.emplace_back(k1 + 1);
                    indices.emplace_back(k2);
                    indices.emplace_back(k2 + 1);
                }
            }
        }


        return {
            std::move(geometry),
            std::move(indices),
        };
    }
}