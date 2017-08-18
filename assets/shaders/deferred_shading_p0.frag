#version 450

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};

layout(location = 10) uniform uint DRAW_ID; // HACK

layout(binding = 0) uniform Camera {
    mat4 view_proj;
    mat4 view;
    mat4 proj;
    mat4 view_proj_inv;
    vec3 position_w;
} CAMERA;

layout(binding = 0) buffer ResourceIndexBuffer {
    uint RESOURCE_INDICES[];
};
layout(binding = 1) buffer MaterialBuffer {
    Material MATERIALS[];
};

in VertexData {
    layout(location = 0) vec3 normal_w;
} IN;

layout(location = 0) out vec4 frag_color0;
layout(location = 1) out vec4 frag_color1;
layout(location = 2) out vec4 frag_color2;
layout(location = 3) out vec4 frag_color3;

void main() {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[DRAW_ID]];

    frag_color0.rgb = normalize(IN.normal_w) * 0.5 + vec3(0.5);
    frag_color0.a = 0.f;
    frag_color1.rgb = MATERIAL.ambient;
    frag_color1.a = 0.f;
    frag_color2.rgb = MATERIAL.diffuse;
    frag_color2.a = 0.f;
    frag_color3.rgb = MATERIAL.specular;
    frag_color3.a = log2(MATERIAL.specular_power) / 10.5;
}
