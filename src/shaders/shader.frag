#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 light_px = vec3(1, 0, 0);
    vec3 light_nx = vec3(-1, 0, 0);
    float col_x = max(0, dot(inNormal, light_px)) + max(0, dot(inNormal, light_nx));

    vec3 light_py = vec3(0, 1, 0);
    vec3 light_ny = vec3(0, -1, 0);
    float col_y = max(0, dot(inNormal, light_py)) + max(0, dot(inNormal, light_ny));

    vec3 light_pz = vec3(0, 0, 1);
    vec3 light_nz = vec3(0, 0, -1);
    float col_z = max(0, dot(inNormal, light_pz)) + max(0, dot(inNormal, light_nz));

    outColor = vec4(0.6 * col_x, 0.6 * col_y, 0.6 * col_z, 1);
}
