#version 450
#extension GL_ARB_separate_shader_objects : enable

struct LightDescription
{
    vec4 color;
    vec4 dir;
};

layout (set = 0, binding = 2) uniform Lights
{
    LightDescription key;
    LightDescription fill;
};

layout(set = 1, binding = 10) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {

    vec4 key  = key.color * max(0, dot(vec3(key.dir), inNormal));
    vec4 fill = fill.color * max(0, dot(vec3(fill.dir), inNormal));

    vec4 light = key + fill;
    outColor = light * texture(texSampler, inTexCoord);
}
