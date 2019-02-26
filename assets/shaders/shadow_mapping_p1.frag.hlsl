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
    float shadow_bias;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);
[[vk::binding(8)]] ConstantBuffer<Debug> DEBUG : register(b8);

// t
[[vk::binding(0)]] StructuredBuffer<uint> RESOURCE_INDICES : register(t0);
[[vk::binding(1)]] StructuredBuffer<Material> MATERIALS : register(t1);
[[vk::binding(2)]] StructuredBuffer<PointLight> LIGHTS : register(t2);
[[vk::binding(3)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t3);
[[vk::binding(8)]] Texture2D<float> SHADOW : register(t8);
SamplerState SAMPLER : register(s8);

// push constant
struct PushConstant {
    uint draw_id;
};
[[vk::push_constant]] PushConstant CONSTANTS;

void main(in PSInput i, out PSOutput o) {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[CONSTANTS.draw_id]];

    float3 final_color;
    if (DEBUG.mode == 1) {  // SHADOWED
        // 影付けが行われる部分を紫色で描画する
        float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[0].view_proj);
        float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
        float depth = SHADOW.Sample(SAMPLER, shadow_coord.xy);
        if (depth < (shadow_coord.z - DEBUG.shadow_bias)) {
            final_color = float3(1.f, 0.f, 1.f);
        } else {
            final_color = float3(1.f, 1.f, 1.f);
        }
    } else {  // DEFAULT
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
                    MATERIAL.diffuse * max(0.f, dot(n, l))
                    +
                    MATERIAL.specular * pow(max(0.f, dot(v, r)), MATERIAL.specular_power)
                ) * LIGHT.color * LIGHT.intensity * atten;
        }

        float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[0].view_proj);
        float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
        float depth = SHADOW.Sample(SAMPLER, shadow_coord.xy);
        if (depth < (shadow_coord.z - DEBUG.shadow_bias)) {
            final_color *= 0.2f;
        }
    }
    o.frag_color = float4(final_color, 1.f);
}
