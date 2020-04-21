#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;
layout (location = 3) in vec3 itangents;
layout (location = 4) in vec3 ibi_tangents;

layout (location = 0) out vec2 uvs;
layout (location = 1) out vec3 normals;
layout (location = 2) out vec3 frag_pos;
layout (location = 3) out vec3 view_pos;

layout (set = 0, binding = 0) uniform Camera {
    mat4 pv;
    vec4 pos;
} camera;

layout (set = 0, binding = 1) buffer readonly Transform {
    mat4[] transforms;
};

layout (push_constant) uniform Indices {
    int transform_index;
    int diffuse_index;
    int specular_index;
    int normal_index;
};

void main() {
    mat4 model = transforms[transform_index];
    uvs = iuvs;
    normals = mat3(transpose(inverse(model))) * inormals;
    frag_pos = vec3(model * vec4(ivertex_pos, 1.0));
    view_pos = vec3(camera.pos);
    gl_Position = camera.pv * vec4(frag_pos, 1.0);
}