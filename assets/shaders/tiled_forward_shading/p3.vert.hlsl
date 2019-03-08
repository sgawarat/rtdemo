/**
 * @brief Tiled Forward Shading - Pass 3: Pos-Pprocessing
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

// 入力
struct VSInput {
    [[vk::location(0)]] float2 position : POSITION;
    // [[vk::builtin("DrawIndex")]] uint draw_id : DRAW_ID;
};

// 出力
struct VSOutput {
    float4 position : SV_Position;
    [[vk::location(0)]] float2 texcoord : TEXCOORD;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

void main(in VSInput i, out VSOutput o) {
    o.position = float4(i.position, 0.f, 1.f);
    o.texcoord = i.position * 0.5f + float2(0.5f, 0.5f);
}
