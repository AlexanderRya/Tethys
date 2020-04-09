#version 460

layout (location = 0) in vec3 ivertex_pos;
layout (location = 1) in vec3 inormals;
layout (location = 2) in vec2 iuvs;

void main() {
    gl_Position = vec4(ivertex_pos, 1.0);
}