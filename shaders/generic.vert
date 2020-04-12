#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;

layout (location = 0) out vec2 uvs;
layout (location = 1) out vec3 normals;

layout (set = 0, binding = 0) uniform Camera {
    mat4 pv_matrix;
};

layout (set = 0, binding = 1) buffer readonly Transform {
    mat4 transform[];
};

layout (push_constant) uniform Indices {
    int transform_index;
    int material_index;
};

void main() {
    gl_Position = pv_matrix * transform[transform_index] * vec4(ivertex_pos, 1.0);
    uvs = iuvs;
    normals = inormals;
}