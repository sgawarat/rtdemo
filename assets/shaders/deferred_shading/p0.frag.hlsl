/**
 * @brief Deferred Shading - Pass 0: Geometry Pass
 *
 * ジオメトリ情報をGバッファに格納する
 */
#include <common.hlsli>
#include <deferred_shading\\common.hlsli>

// 入力
struct PSInput {
  [[vk::location(0)]] float3 normal_w : NORMAL_W;
  // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
  [[vk::location(0)]] float4 frag_color0 : SV_Target0;  // RT0
  [[vk::location(1)]] float4 frag_color1 : SV_Target1;  // RT1
  [[vk::location(2)]] float4 frag_color2 : SV_Target2;  // RT2
  [[vk::location(3)]] float4 frag_color3 : SV_Target3;  // RT3
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<uint> RESOURCE_INDICES : register(t0);
[[vk::binding(1)]] StructuredBuffer<Material> MATERIALS : register(t1);

void main(in PSInput i, out PSOutput o) {
  const uint resource_index = RESOURCE_INDICES[G.draw_id];
  const Material material = MATERIALS[resource_index];

  // シェーディングを行うために必要な情報をレンダターゲットに格納する
  // 位置は深度値から再構築するので格納する必要はない
  o.frag_color0.rgb = encode_normal(normalize(i.normal_w));
  o.frag_color0.a = 0.f;
  o.frag_color1.rgb = material.ambient;
  o.frag_color1.a = 0.f;
  o.frag_color2.rgb = material.diffuse;
  o.frag_color2.a = 0.f;
  o.frag_color3.rgb = material.specular;
  o.frag_color3.a = encode_specular_power(material.specular_power);
}
