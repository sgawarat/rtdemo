/**
 * @brief Tiled Forward Shading - Pass 2: Shading
 *
 * 前パスで生成されたライト番号のリストを用いて、通常のForward Shadingを行う
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

// 入力
struct VSInput {
    [[vk::location(0)]] float3 position : POSITION;  // 位置
    [[vk::location(1)]] float3 normal : NORMAL;  // 法線
    // [[vk::builtin("DrawIndex")]] uint draw_id : DrawId;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float3 position_w : POSITION_W;  // ワールド空間の位置
    [[vk::location(1)]] float3 normal_w : NORMAL_W;  // ワールド空間の法線
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

void main(in VSInput i, out VSOutput o) {
    float4 position_c = mul(float4(i.position, 1.f), CAMERA.view_proj);  // クリップ空間の位置

    o.position = position_c;
    o.position_w = i.position;
    o.normal_w = i.normal;
}
