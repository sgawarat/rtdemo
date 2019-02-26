#include "detail/types.hlsli"

// 入力
struct PSInput {
    [[vk::location(0)]] float3 position_w : POSITION_W;
    [[vk::location(1)]] float3 normal_w : NORMAL_W;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
    [[vk::location(0)]] float4 frag_color : SV_Target;
};

// デバッグ用
struct Debug {
    int mode;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);
[[vk::binding(8)]] ConstantBuffer<Debug> DEBUG : register(b8);

// t
[[vk::binding(0)]] StructuredBuffer<uint> RESOURCE_INDICES : register(t0);
[[vk::binding(1)]] StructuredBuffer<Material> MATERIALS : register(t1);
[[vk::binding(2)]] StructuredBuffer<PointLight> LIGHTS : register(t2);

// push constant
struct PushConstant {
    uint draw_id;
};
[[vk::push_constant]] PushConstant CONSTANTS;

void main(in PSInput i, out PSOutput o) {
    // 描画IDが指定するマテリアルを取得する
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[CONSTANTS.draw_id]];

    float3 final_color;
    if (DEBUG.mode == 1) {  // 位置
        final_color = abs(i.position_w) % float3(1.f, 1.f, 1.f);
    } else if (DEBUG.mode == 2) {  // 法線
        final_color = normalize(i.normal_w) * 0.5f + float3(0.5f, 0.5f, 0.5f);
    } else if (DEBUG.mode == 3) {  // アンビエント
        final_color = MATERIAL.ambient;
    } else if (DEBUG.mode == 4) {  // ディフューズ
        final_color = MATERIAL.diffuse;
    } else if (DEBUG.mode == 5) {  // スペキュラ
        final_color = MATERIAL.specular;
    } else if (DEBUG.mode == 6) {  // スペキュラパワー
        final_color = float1(log2(MATERIAL.specular_power) / 10.5f).xxx;
    } else {  // 通常
        float3 v = normalize(CAMERA.position_w - i.position_w);
        float3 n = normalize(i.normal_w);

        final_color = float3(0.f, 0.f, 0.f);//MATERIAL.ambient;
        int lights_length = 1;//LIGHTS.length();
        for (int k = 0; k < lights_length; ++k) {
            PointLight LIGHT = LIGHTS[k];

            float3 lv = LIGHT.position_w - i.position_w;
            float l_len = length(lv);
            float atten = 1.f;
            if (l_len >= LIGHT.radius) atten = 0.f;
            float3 l = (LIGHT.position_w - i.position_w) / l_len;
            float3 r = reflect(-l, n);

            final_color +=
                (
                    MATERIAL.diffuse * max(0, dot(n, l))
                    +
                    MATERIAL.specular * pow(max(0, dot(v, r)), MATERIAL.specular_power)
                ) * LIGHT.color * LIGHT.intensity * atten;
        }
    }
    o.frag_color = float4(final_color, 1.f);
}
