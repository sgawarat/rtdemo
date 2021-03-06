﻿/**
 * Forward Shading
 */
#include <common.hlsli>
#include <forward_shading\\common.hlsli>

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

void main(in VSInput i, out VSOutput o) {
  // クリップ空間の位置を計算する
  float4 position_c = mul(float4(i.position, 1.f), CAMERA.view_proj);

  // 出力する
  o.position = position_c;
  o.position_w = i.position;
  o.normal_w = i.normal;
}
