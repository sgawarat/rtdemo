#version 450

struct Camera {
    mat4 view_proj;
    mat4 view;
    mat4 proj;
    mat4 view_proj_inv;
    mat4 view_inv;
    mat4 proj_inv;
    vec3 position_w;
};
layout(binding = 0) uniform CameraUniform {
    Camera CAMERA;
};

layout(binding = 0) buffer ResourceIndexBuffer {
    uint RESOURCE_INDICES[];
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};
layout(binding = 1) buffer MaterialBuffer {
    Material MATERIALS[];
};

struct PointLight {
    vec3 position_w;
    float radius;
    vec3 color;
    float intensity;
};
layout(binding = 2) buffer LightBuffer {
    PointLight LIGHTS[];
};

struct ShadowCaster {
    mat4 view_proj;
};
layout(binding = 3) buffer ShadowCasterBuffer {
    ShadowCaster SHADOW_CASTERS[];
};

layout(binding = 8) uniform sampler2D DEPTH;

in VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} IN;

layout(location = 0) out vec4 frag_color;

layout(location = 0) uniform uint DRAW_ID;
layout(location = 32) uniform uint DEBUG_VIEW;
layout(location = 33) uniform float SHADOW_BIAS;

void main() {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[DRAW_ID]];

    vec3 final_color;
    if (DEBUG_VIEW == 1) {  // SHADOWED
        // 影付けが行われる部分を紫色で描画する
        vec4 position_sm = SHADOW_CASTERS[0].view_proj * vec4(IN.position_w, 1.f);
        vec3 shadow_coord = (position_sm.xyz / position_sm.w) * vec3(0.5f) + vec3(0.5f);
        float depth = texture(DEPTH, shadow_coord.xy).r;
        // final_color = vec3((depth - 0.98f) / 0.02f);
        if (depth < (shadow_coord.z - SHADOW_BIAS)) {
            final_color = vec3(1.f, 0.f, 1.f);
        } else {
            final_color = vec3(1.f, 1.f, 1.f);
        }
    } else {  // DEFAULT
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

        vec4 position_sm = SHADOW_CASTERS[0].view_proj * vec4(IN.position_w, 1.f);
        vec3 shadow_coord = (position_sm.xyz / position_sm.w) * vec3(0.5f) + vec3(0.5f);
        float depth = texture(DEPTH, shadow_coord.xy).r;
        if (depth < (shadow_coord.z - SHADOW_BIAS)) {
            final_color *= 0.2f;
        }
    }
    frag_color = vec4(final_color, 1);
}
