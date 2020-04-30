#version 460
#extension GL_EXT_nonuniform_qualifier : enable

void main() {
    gl_FragDepth = gl_FragCoord.z;
}
