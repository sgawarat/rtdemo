/**
 * @brief Forward Shading
 */
#include <common.hlsli>
#include <forward_shading\\common.hlsli>

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

void main(in PSInput i, out PSOutput o) {
  // 描画IDが指定するマテリアルを取得する
  const uint resource_index = RESOURCE_INDICES[G.draw_id];
  const Material material = MATERIALS[resource_index];

  float3 final_color;
  switch (MODE) {
  case 0: {  // 通常
    float3 v = normalize(CAMERA.position_w - i.position_w);
    float3 n = normalize(i.normal_w);

    final_color = float3(0.f, 0.f, 0.f);//MATERIAL.ambient;
    for (uint k = 0; k < LIGHT_COUNT; ++k) {
      const PointLight light = LIGHTS[k];

      const float3 lv = light.position_w - i.position_w;  // シェーディングポイントからライト位置へのベクトル
      const float l_len = length(lv);  // ライトまでの距離
      const float3 l = (light.position_w - i.position_w) / l_len;  // ライト方向
      const float3 r = reflect(-l, n);  // ライト方向の反射ベクトル

      // 減衰率を計算する
      // TODO:ちゃんとした減衰率を計算する
      float atten = 1.f;
      if (l_len >= light.radius) atten = 0.f;

      // Phongっぽく計算する
      final_color +=
          (
              material.diffuse * max(0, dot(n, l))
              +
              material.specular * pow(max(0, dot(v, r)), material.specular_power)
          ) * light.color * light.intensity * atten;
    }
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
  }

  o.frag_color = float4(final_color, 1.f);
}
