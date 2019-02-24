#version 450

// カメラ
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

// リソース番号
layout(binding = 0) buffer ResourceIndexBuffer {
    uint RESOURCE_INDICES[];
};

// マテリアル
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};
layout(binding = 1) buffer MaterialBuffer {
    Material MATERIALS[];
};

// ライト
struct PointLight {
    vec3 position_w;
    float radius;
    vec3 color;
    float intensity;
};
layout(binding = 2) buffer LightBuffer {
    PointLight LIGHTS[];
};

// 前段から受け取る値
in VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} IN;

// 出力する値
layout(location = 0) out vec4 frag_color;

// Scene由来の定数
layout(location = 0) uniform uint DRAW_ID;

// Technique由来の定数
layout(location = 32) uniform uint DEBUG_VIEW;

void main() {
    // 描画IDが指定するマテリアルを取得する
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[DRAW_ID]];

    vec3 final_color;
    if (DEBUG_VIEW == 1) {  // 位置
        final_color = mod(abs(IN.position_w), vec3(1));
    } else if (DEBUG_VIEW == 2) {  // 法線
        final_color = normalize(IN.normal_w) * 0.5 + vec3(0.5);
    } else if (DEBUG_VIEW == 3) {  // アンビエント
        final_color = MATERIAL.ambient;
    } else if (DEBUG_VIEW == 4) {  // ディフューズ
        final_color = MATERIAL.diffuse;
    } else if (DEBUG_VIEW == 5) {  // スペキュラ
        final_color = MATERIAL.specular;
    } else if (DEBUG_VIEW == 6) {  // スペキュラパワー
        final_color = vec3(log2(MATERIAL.specular_power) / 10.5);
    } else {  // 通常
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
