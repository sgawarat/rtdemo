/**
 * Shadow Mapping - Pass 0: シャドウマップの生成
 */
#include <common.hlsli>
#include <shadow_mapping\\common.hlsli>

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

void main(in VSInput i, out VSOutput o) {
  const float4x4 view_proj = SHADOW_CASTERS[SHADOW_CASTER_INDEX].view_proj;
  float4 position_c = mul(float4(i.position, 1.f), view_proj);

  o.position = position_c;
}
