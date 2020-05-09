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
    mat3 TBN;
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
    uint specular_index;
    uint normal_index;
    uint point_lights_count;
    uint directional_lights_count;
};

void main() {
    mat4 model = transforms[transform_index];

    vec3 T = normalize(vec3(model * vec4(itangents, 0.0)));
    vec3 B = normalize(vec3(model * vec4(ibi_tangents, 0.0)));
    vec3 N = normalize(vec3(model * vec4(inormals, 0.0)));

    TBN = mat3(T, B, N);
    vertex_pos = ivertex_pos;
    frag_pos = vec3(model * vec4(ivertex_pos, 1.0));
    uvs = iuvs;
    view_pos = vec3(camera.pos);
    normals = inormals;
    gl_Position = camera.proj * camera.view * vec4(frag_pos, 1.0);
}