/**
 * Shadow Mapping - Pass 1: シェーディング
 */
#include <common.hlsli>
#include <shadow_mapping\\common.hlsli>

// 入力
struct PSInput {
    [[vk::location(0)]] float3 position_w : POSITION_W;
    [[vk::location(1)]] float3 normal_w : NORMAL_W;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
    [[vk::location(0)]] float4 frag_color : SV_Target;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<uint> RESOURCE_INDICES : register(t0);
[[vk::binding(1)]] StructuredBuffer<Material> MATERIALS : register(t1);
[[vk::binding(2)]] StructuredBuffer<PointLight> LIGHTS : register(t2);
[[vk::binding(3)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t3);
[[vk::binding(8)]] Texture2D<float> SHADOW : register(t8);
SamplerState SAMPLER : register(s8);

void main(in PSInput i, out PSOutput o) {
  const uint resource_index = RESOURCE_INDICES[G.draw_id];
  const Material material = MATERIALS[resource_index];

  float3 final_color;
  switch (MODE) {
  case 0: {  // 通常
    const float3 v = normalize(CAMERA.position_w - i.position_w);
    const float3 n = normalize(i.normal_w);

    // シェーディング
    final_color = float3(0.f, 0.f, 0.f);//MATERIAL.ambient;
    // for (int k = 0; k < LIGHT_COUNT; ++k) {
    const int k = SHADOW_CASTER_INDEX;
    {
      // シャドウ
      const float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[k].view_proj);
      const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
      const float shadow_depth = SHADOW.Sample(SAMPLER, shadow_coord.xy);
      float shadow;
      if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
        shadow = 0.f;
      } else {
        shadow = 1.f;
      }

      // ライティング
      const PointLight light = LIGHTS[k];

      const float3 lv = light.position_w - i.position_w;
      const float l_len = length(lv);
      const float3 l = (light.position_w - i.position_w) / l_len;
      const float3 r = reflect(-l, n);

      float atten = 1.f;
      if (l_len >= light.radius) atten = 0.f;

      final_color +=
          (
              material.diffuse * max(0.f, dot(n, l))
              +
              material.specular * pow(max(0.f, dot(v, r)), material.specular_power)
          ) * light.color * light.intensity * atten * shadow;
    }

    break;
  }
  case 1: {  // 影付けされた部分を色付けする
    const int k = SHADOW_CASTER_INDEX;
    const float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[k].view_proj);
    const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
    const float shadow_depth = SHADOW.Sample(SAMPLER, shadow_coord.xy);
    if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
        final_color = float3(1.f, 0.f, 1.f);
    } else {
        final_color = float3(1.f, 1.f, 1.f);
    }
    break;
  }
  }

  o.frag_color = float4(final_color, 1.f);
}
