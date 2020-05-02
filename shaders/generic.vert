#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;
layout (location = 3) in vec3 itangents;
layout (location = 4) in vec3 ibi_tangents;

layout (location = 0) out vec3 vertex_pos;
layout (location = 1) out vec3 frag_pos;
layout (location = 2) out vec3 normals;
layout (location = 3) out vec2 uvs;
layout (location = 4) out vec3 view_pos;
layout (location = 5) out vec4 shadow_frag_pos;

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
    mat4 model = transforms[transform_index];

    vertex_pos = ivertex_pos;
    frag_pos = vec3(model * vec4(ivertex_pos, 1.0));
    normals = transpose(inverse(mat3(model))) * inormals;
    uvs = iuvs;
    view_pos = vec3(camera.pos);
    shadow_frag_pos = light_space * vec4(frag_pos, 1.0);
    gl_Position = camera.pv * vec4(frag_pos, 1.0);
}