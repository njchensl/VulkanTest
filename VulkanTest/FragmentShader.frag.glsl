#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 color;

void main() {
    color = vec4(2.0f, 3.0f, 8.0f, 1.0f);
}
