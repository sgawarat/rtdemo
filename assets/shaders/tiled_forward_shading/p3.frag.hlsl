/**
 * @brief Tiled Forward Shading - Pass 3: Pos-Pprocessing
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

// 入力
struct PSInput {
    [[vk::location(0)]] float2 texcoord : TEXCOORD;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
    [[vk::location(0)]] float4 frag_color : SV_Target;
};

// t
[[vk::binding(8)]] Texture2D<float4> RT0 : register(t8);

void main(in PSInput i, out PSOutput o) {
    uint2 size;
    RT0.GetDimensions(size.x, size.y);
    o.frag_color = RT0.Load(int3(i.texcoord * size, 0));
}
