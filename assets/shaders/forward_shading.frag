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
    mat4 view_proj;
    mat4 view_proj_inv;
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

in VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} IN;
layout(location = 10) uniform uint draw_id; // HACK
layout(location = 0) out vec4 frag_color;
void main() {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[draw_id]];
    Light LIGHT = LIGHTS[0];

    vec3 l = normalize(LIGHT.position_w - IN.position_w);
    vec3 v = normalize(CAMERA.position_w - IN.position_w);
    vec3 n = normalize(IN.normal_w);
    vec3 r = reflect(-l, n);

    vec3 final_color =
        MATERIAL.ambient
        +
        MATERIAL.diffuse * max(0, dot(n, l))
        +
        MATERIAL.specular * pow(max(0, dot(v, r)), MATERIAL.specular_power)
        ;
    frag_color = vec4(final_color, 1);
}
