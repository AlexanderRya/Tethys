#version 460
#extension GL_EXT_nonuniform_qualifier : enable

const float ambient = 0.1;

layout (location = 0) in vertex_out {
    vec3 vertex_pos;
    vec3 frag_pos;
    vec2 uvs;
    vec3 view_pos;
    vec3 normals;
    mat3 TBN;
};

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

layout (std140, set = 1, binding = 0) buffer readonly PointLights {
    PointLight[] point_lights;
};

layout (std140, set = 1, binding = 1) buffer readonly DirectionalLights {
    DirectionalLight[] directional_lights;
};

layout (push_constant) uniform Constants {
    uint transform_index;
    uint albedo_index;
    uint specular_index;
    uint normal_index;
    uint point_lights_count;
    uint directional_lights_count;
};

void main() {
    vec3 result = vec3(1.0);

    vec3 albedo = texture(textures[albedo_index], uvs).rgb;
    vec3 specular = texture(textures[specular_index], uvs).rgb;
    vec3 normal =  texture(textures[normal_index], uvs).rgb;

    result = albedo * ambient;

    frag_color = vec4(result, 1.0);
}