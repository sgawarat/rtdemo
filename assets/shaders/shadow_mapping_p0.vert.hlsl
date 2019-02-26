#include "detail/types.hlsli"

// 入力
struct VSInput {
    [[vk::location(0)]] float3 position : POSITION;
    // [[vk::builtin("DrawIndex")]] uint draw_id : DRAW_ID;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// t
[[vk::binding(0)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t0);

// push constant
struct PushConstant {
    uint draw_id;
};
[[vk::push_constant]] PushConstant CONSTANTS;

void main(in VSInput i, out VSOutput o) {
    float4 position_c = mul(float4(i.position, 1.f), SHADOW_CASTERS[0].view_proj);

    o.position = position_c;
}
