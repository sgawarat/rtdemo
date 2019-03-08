/**
 * @brief Tiled Forward Shading - Pass 0: Depth Pre-Pass
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

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

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

void main(in VSInput i, out VSOutput o) {
    o.position = mul(float4(i.position, 1.f), CAMERA.view_proj);
}
