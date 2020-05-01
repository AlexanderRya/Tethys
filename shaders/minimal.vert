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

layout (push_constant) uniform Constants {
    uint transform_index;
    uint material_index;
};

void main() {
    uvs = iuvs;
    gl_Position = camera.pv * transforms[transform_index] * vec4(ivertex_pos, 1.0);
}