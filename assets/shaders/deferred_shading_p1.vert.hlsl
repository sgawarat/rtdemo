#include <common.hlsli>

// 入力
struct VSInput {
    [[vk::location(0)]] float2 position : POSITION;
    // [[vk::builtin("DrawIndex")]] uint draw_id : DRAW_ID;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 position_ndc : POSITION_NDC;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// デバッグ用
struct Debug {
    int mode;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);
[[vk::binding(8)]] ConstantBuffer<Debug> DEBUG : register(b8);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);
[[vk::binding(8)]] Texture2D<float> DEPTH : register(t8);
[[vk::binding(9)]] Texture2D<float4> GBUFFER0 : register(t9);
[[vk::binding(10)]] Texture2D<float4> GBUFFER1 : register(t10);
[[vk::binding(11)]] Texture2D<float4> GBUFFER2 : register(t11);
[[vk::binding(12)]] Texture2D<float4> GBUFFER3 : register(t12);
SamplerState SAMPLER : register(s0);

void main(in VSInput i, out VSOutput o) {
    PointLight LIGHT = LIGHTS[G.draw_id];

    float4 position_c = float4(i.position, 0.f, 1.f);

    o.position = position_c;
    o.position_ndc = position_c.xyz / position_c.w;
    // o.draw_id = i.draw_id;
}
