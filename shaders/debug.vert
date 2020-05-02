#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;
layout (location = 3) in vec3 itangents;
layout (location = 4) in vec3 ibi_tangents;

layout (location = 0) out vec2 uvs;

layout (set = 0, binding = 0) uniform Camera {
    mat4 pv;
    vec4 pos;
} camera;

layout (set = 0, binding = 1) buffer readonly Transform {
    mat4[] transforms;
};

layout (set = 1, binding = 3) uniform LightSpace {
    mat4 light_space;
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
    uvs = iuvs;
    gl_Position = vec4(ivertex_pos, 1.0);
}