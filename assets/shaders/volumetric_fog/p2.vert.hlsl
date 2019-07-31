/**
 * Volumetric Fog - Pass 2: Rendering
 */
#include "common.hlsli"

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
  [[vk::location(2)]] float3 position_v : POSITION_V;
  [[vk::location(3)]] float4 position_ch : POSITION_CH;
  // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

void main(in VSInput i, out VSOutput o) {
  // 各空間の位置を計算する
  float3 position_v = mul(float4(i.position, 1.f), CAMERA.view).xyz;
  float4 position_ch = mul(float4(i.position, 1.f), CAMERA.view_proj);
  float3 position_c = position_ch.xyz / position_ch.w;

  // 出力する
  o.position = position_ch;
  o.position_w = i.position;
  o.normal_w = i.normal;
  o.position_v = position_v;
  o.position_ch = position_ch;
}
