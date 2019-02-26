#include "detail/types.hlsli"

// 入力
struct VSInput {
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
    // [[vk::builtin("DrawIndex")]] uint draw_id : DrawId;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 normal_w : NORMAL_W;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

void main(in VSInput i, out VSOutput o) {
    o.position = mul(float4(i.position, 1.f), CAMERA.view_proj);
    o.normal_w = i.normal;
    // o.draw_id = i.draw_id;
}
