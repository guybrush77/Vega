#version 450
#extension GL_ARB_separate_shader_objects : enable

struct LightDescription
{
    vec4 color;
    vec4 dir;
};

layout (binding = 10) uniform Lights
{
    LightDescription key;
    LightDescription fill;
};


layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {

    vec4 key  = key.color * max(0, dot(vec3(key.dir), inNormal));
    vec4 fill = fill.color * max(0, dot(vec3(fill.dir), inNormal));

    outColor = key + fill;
}
