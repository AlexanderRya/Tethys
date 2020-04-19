#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) in vec2 uvs;

layout (location = 0) out vec4 frag_color;

layout (set = 0, binding = 2) uniform sampler2D[] textures;

layout (push_constant) uniform Indices {
    int transform_index;
    int material_index;
};

void main() {
    frag_color = vec4(texture(textures[material_index], uvs).rgb, 1.0);
}
