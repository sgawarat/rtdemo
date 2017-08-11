#version 450
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};
struct Light {
    vec3 position_w;
};

layout(binding = 0) uniform Camera {
    mat4 wvp;
    vec3 position_w;
} CAMERA;
layout(binding = 0) buffer ResourceIndexBuffer {
    uint RESOURCE_INDICES[];
};
layout(binding = 1) buffer MaterialBuffer {
    Material MATERIALS[];
};
layout(binding = 2) buffer LightBuffer {
    Light LIGHTS[];
};

layout(binding = 0) uniform sampler2D DEPTH;
layout(binding = 1) uniform sampler2D GBUFFER0;
layout(binding = 2) uniform sampler2D GBUFFER1;
layout(binding = 3) uniform sampler2D GBUFFER2;
layout(binding = 4) uniform sampler2D GBUFFER3;

in VertexData {
    layout(location = 0) vec2 texcoord;
} IN;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 normal_w = texture(GBUFFER0, IN.texcoord).xyz;
    vec3 n = normalize(normal_w);

    frag_color = vec4(n, 1);
}
