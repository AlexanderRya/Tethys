#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;
layout (location = 3) in vec3 itangents;
layout (location = 4) in vec3 ibi_tangents;

layout (location = 0) out vertex_out {
    vec3 vertex_pos;
    vec3 frag_pos;
    vec2 uvs;
    vec3 view_pos;
    vec3 normals;
};

layout (set = 0, binding = 0) uniform Camera {
    mat4 proj;
    mat4 view;
    vec4 pos;
} camera;

layout (set = 0, binding = 1) buffer readonly Transform {
    mat4[] transforms;
};

layout (push_constant) uniform Constants {
    uint transform_index;
    uint albedo_index;
    uint metallic_index;
    uint normal_index;
    uint roughness_index;
    uint occlusion_index;
    uint point_lights_count;
    uint directional_lights_count;
};

void main() {
    mat4 model = transforms[transform_index];

    vertex_pos = ivertex_pos;
    frag_pos = vec3(model * vec4(ivertex_pos, 1.0));
    uvs = iuvs;
    view_pos = vec3(camera.pos);
    normals = mat3(model) * inormals;
    gl_Position = camera.proj * camera.view * vec4(frag_pos, 1.0);
}