#version 460
#extension GL_EXT_nonuniform_qualifier : enable

const float ambient = 0.1;

layout (location = 0) in vec2 uvs;

layout (location = 0) out vec4 frag_color;

struct Falloff {
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct PointLight {
    vec3 position;
    vec3 color;
    Falloff falloff;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

layout (set = 0, binding = 2) uniform sampler2D[] textures;
layout (set = 1, binding = 0) uniform sampler2D shadow_map;

layout (std140, set = 1, binding = 1) buffer readonly PointLights {
    PointLight[] point_lights;
};

layout (std140, set = 1, binding = 2) buffer readonly DirectionalLights {
    DirectionalLight[] directional_lights;
};

layout (push_constant) uniform Constants {
    uint transform_index;
    uint diffuse_index;
    uint specular_index;
    uint normal_index;
    uint point_lights_count;
    uint directional_lights_count;
};

void main() {
    frag_color = vec4(vec3(1.0 - (1.0 - texture(shadow_map, uvs).r) * 25.0), 1.0);
}
