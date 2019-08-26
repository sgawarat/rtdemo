/**
 * @brief Volumetric Fog - Pass 2: Rendering
 */
#include "common.hlsli"

// 入力
struct PSInput {
  float4 position : SV_Position;
  [[vk::location(0)]] float3 position_w : POSITION_W;
  [[vk::location(1)]] float3 normal_w : NORMAL_W;
  [[vk::location(2)]] float3 position_v : POSITION_V;
  [[vk::location(3)]] float4 position_ch : POSITION_CH;
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
[[vk::binding(8)]] Texture3D<float4> VLIGHTING : register(t8);
[[vk::binding(9)]] Texture2D<float> SHADOW : register(t9);
SamplerState VLIGHTING_SAMPLER : register(s8);
SamplerState SHADOW_SAMPLER : register(s9);

float3 shading(PSInput i, Material material) {
    const float3 v = normalize(CAMERA.position_w - i.position_w);
    const float3 n = normalize(i.normal_w);

    float3 final_color = float3(0.f, 0.f, 0.f);//MATERIAL.ambient;

    for (uint k = 0; k < 1; ++k) {
      const PointLight light = LIGHTS[k];

      const float3 lv = light.position_w;  // ライトの向きのベクトル
      const float l_len = length(lv);  // ライトまでの距離
      const float3 l = light.position_w / l_len;  // ライト方向
      const float3 r = reflect(-l, n);  // ライト方向の反射ベクトル

      // 減衰率を計算する
      const float atten = 1.f;//calc_attenuation(l_len);

      // シャドウを計算する
      float visibility = 1.f;
      if (k == 0) {  // TODO:シャドウを持たないライトに対応する
        const float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[k].view_proj);
        const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
        const float shadow_depth = SHADOW.Sample(SHADOW_SAMPLER, shadow_coord.xy);
        if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
          visibility = 0.f;
        } else {
          visibility = 1.f;
        }
      }

      // Phongっぽく計算する
      final_color +=
          (
              material.diffuse * max(0, dot(n, l))
              +
              material.specular * pow(max(0, dot(v, r)), material.specular_power)
          ) * light.color * light.intensity * atten * visibility;
    }

    for (; k < LIGHT_COUNT; ++k) {
      const PointLight light = LIGHTS[k];

      const float3 lv = light.position_w - i.position_w;  // シェーディングポイントからライト位置へのベクトル
      const float l_len = length(lv);  // ライトまでの距離
      const float3 l = (light.position_w - i.position_w) / l_len;  // ライト方向
      const float3 r = reflect(-l, n);  // ライト方向の反射ベクトル

      // 減衰率を計算する
      const float atten = calc_attenuation(l_len);

      // シャドウを計算する
      float visibility = 1.f;
      if (k == 0) {  // TODO:シャドウを持たないライトに対応する
        const float4 position_sm = mul(float4(i.position_w, 1.f), SHADOW_CASTERS[k].view_proj);
        const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
        const float shadow_depth = SHADOW.Sample(SHADOW_SAMPLER, shadow_coord.xy);
        if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
          visibility = 0.f;
        } else {
          visibility = 1.f;
        }
      }

      // Phongっぽく計算する
      final_color +=
          (
              material.diffuse * max(0, dot(n, l))
              +
              material.specular * pow(max(0, dot(v, r)), material.specular_power)
          ) * light.color * light.intensity * atten * visibility;
    }

    return final_color;
}

void main(in PSInput i, out PSOutput o) {
  // 描画IDが指定するマテリアルを取得する
  const uint resource_index = RESOURCE_INDICES[G.draw_id];
  const Material material = MATERIALS[resource_index];

  // ボリューメトリックライティングの結果を取り出す
  float3 position_vol = convert_from_view_to_volume(i.position_v, CAMERA);
  float3 texcoord_vol = convert_from_position_to_texcoord(position_vol);
  float4 vlighting = VLIGHTING.Sample(VLIGHTING_SAMPLER, texcoord_vol);

  float3 final_color;
  switch (MODE) {
  case 0: {  // 通常
    final_color = shading(i, material);
    break;
  }
  case 1: {  // 位置
    final_color = abs(i.position_w) % float3(1.f, 1.f, 1.f);
    break;
  }
  case 2: {  // 法線
    final_color = normalize(i.normal_w) * 0.5f + float3(0.5f, 0.5f, 0.5f);
    break;
  }
  case 3: {  // アンビエント
    final_color = material.ambient;
    break;
  }
  case 4: {  // ディフューズ
    final_color = material.diffuse;
    break;
  }
  case 5: {  // スペキュラ
    final_color = material.specular;
    break;
  }
  case 6: {  // スペキュラパワー
    final_color = float1(log2(material.specular_power) / 10.5f).xxx;
    break;
  }
  case 7: {  // Vバッファ用のテクスチャ座標
    final_color = texcoord_vol;
    break;
  }
  case 8: {  // 散乱
    final_color = vlighting.rgb;
    break;
  }
  case 9: {  // 透過率
    final_color = vlighting.a;
    break;
  }
  case 10: {  // ボリューメトリックライティング付き
    final_color = shading(i, material);

    // ボリューメトリックライティング結果を反映する
    if (MODE == 10) {
      final_color *= vlighting.a;
      final_color += vlighting.rgb;
    }

    break;
  }
  }

  o.frag_color = float4(final_color, 1.f);
}
