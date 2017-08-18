#version 450

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};
struct PointLight {
    vec3 position_w;
    float intensity;
    vec3 color;
    float radius;
};

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
layout(binding = 2) buffer LightBuffer {
    PointLight LIGHTS[];
};

in VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} IN;
layout(location = 10) uniform uint DRAW_ID;
layout(location = 11) uniform uint DEBUG_VIEW;
layout(location = 0) out vec4 frag_color;
void main() {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[DRAW_ID]];

    vec3 final_color;
    if (DEBUG_VIEW == 1) {
        final_color = mod(abs(IN.position_w), vec3(1));
    } else if (DEBUG_VIEW == 2) {
        final_color = normalize(IN.normal_w) * 0.5 + vec3(0.5);
    } else if (DEBUG_VIEW == 3) {
        final_color = MATERIAL.ambient;
    } else if (DEBUG_VIEW == 4) {
        final_color = MATERIAL.diffuse;
    } else if (DEBUG_VIEW == 5) {
        final_color = MATERIAL.specular;
    } else if (DEBUG_VIEW == 6) {
        final_color = vec3(log2(MATERIAL.specular_power) / 10.5);
    } else {
        vec3 v = normalize(CAMERA.position_w - IN.position_w);
        vec3 n = normalize(IN.normal_w);

        final_color = vec3(0);//MATERIAL.ambient;
        int lights_length = LIGHTS.length();
        for (int i = 0; i < lights_length; ++i) {
            PointLight LIGHT = LIGHTS[i];

            vec3 lv = LIGHT.position_w - IN.position_w;
            float l_len = length(lv);
            float atten = 1.f;
            if (l_len >= LIGHT.radius) atten = 0.f;
            vec3 l = (LIGHT.position_w - IN.position_w) / l_len;
            vec3 r = reflect(-l, n);

            final_color +=
                (
                    MATERIAL.diffuse * max(0, dot(n, l))
                    +
                    MATERIAL.specular * pow(max(0, dot(v, r)), MATERIAL.specular_power)
                ) * LIGHT.color * LIGHT.intensity * atten;
        }
    }
    frag_color = vec4(final_color, 1);
}
