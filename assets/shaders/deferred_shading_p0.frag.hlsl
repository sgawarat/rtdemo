#include "detail/types.hlsli"

// 入力
struct PSInput {
    [[vk::location(0)]] float3 normal_w : NORMAL_W;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
    [[vk::location(0)]] float4 frag_color0 : SV_Target0;  // RT0
    [[vk::location(1)]] float4 frag_color1 : SV_Target1;  // RT1
    [[vk::location(2)]] float4 frag_color2 : SV_Target2;  // RT2
    [[vk::location(3)]] float4 frag_color3 : SV_Target3;  // RT3
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<uint> RESOURCE_INDICES : register(t0);
[[vk::binding(1)]] StructuredBuffer<Material> MATERIALS : register(t1);

// push constant
struct PushConstant {
    uint draw_id;
};
[[vk::push_constant]] PushConstant CONSTANTS;


void main(in PSInput i, out PSOutput o) {
    Material MATERIAL = MATERIALS[RESOURCE_INDICES[CONSTANTS.draw_id]];

    o.frag_color0.rgb = normalize(i.normal_w) * 0.5f + float3(0.5f, 0.5f, 0.5f);
    o.frag_color0.a = 0.f;
    o.frag_color1.rgb = MATERIAL.ambient;
    o.frag_color1.a = 0.f;
    o.frag_color2.rgb = MATERIAL.diffuse;
    o.frag_color2.a = 0.f;
    o.frag_color3.rgb = MATERIAL.specular;
    o.frag_color3.a = log2(MATERIAL.specular_power) / 10.5f;
}
