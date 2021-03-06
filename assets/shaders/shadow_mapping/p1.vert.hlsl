﻿/**
 * Shadow Mapping - Pass 1: シェーディング
 */
#include <common.hlsli>
#include <shadow_mapping\\common.hlsli>

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

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(3)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t3);

void main(in VSInput i, out VSOutput o) {
  const float4 position_c = mul(float4(i.position, 1.f), CAMERA.view_proj);

  o.position = position_c;
  o.position_w = i.position;
  o.normal_w = i.normal;
}
