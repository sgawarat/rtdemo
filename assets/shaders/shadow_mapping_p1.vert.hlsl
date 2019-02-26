#include "detail/types.hlsli"

// 入力
struct VSInput {
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
    // [[vk::builtin("DrawIndex")]] uint draw_id : DRAW_ID;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 position_w : POSITION_W;
    [[vk::location(1)]] float3 normal_w : NORMAL_W;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
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
[[vk::binding(3)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t3);

void main(in VSInput i, out VSOutput o) {
    float4 position_c;

    if (DEBUG.mode == 2) {  // CASTER
        position_c = mul(float4(i.position, 1.f), SHADOW_CASTERS[0].view_proj);
    } else {
        position_c = mul(float4(i.position, 1.f), CAMERA.view_proj);
    }

    o.position = position_c;
    o.position_w = i.position;
    o.normal_w = i.normal;
}
