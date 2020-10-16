#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 light = vec3(-0.3, 0.7, 0.7);
    float col = max(0, dot(inNormal, light));
    outColor = vec4(0.9 * col, 0.7 * col, 0.5 * col, 1);
}
