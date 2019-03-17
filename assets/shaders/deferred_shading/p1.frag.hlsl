/**
 * @brief Deferred Shading - Pass 1: Lighting Pass
 *
 * Gバッファを使ってシェーディングを行う
 */
#include <common.hlsli>
#include <deferred_shading\\common.hlsli>

// 入力
struct PSInput {
  [[vk::location(0)]] float3 position_ndc : POSITION_NDC;  // NDCでの位置
  // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
  [[vk::location(0)]] float4 frag_color : SV_Target;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);
[[vk::binding(8)]] Texture2D<float> DEPTH : register(t8);
[[vk::binding(9)]] Texture2D<float4> GBUFFER0 : register(t9);
[[vk::binding(10)]] Texture2D<float4> GBUFFER1 : register(t10);
[[vk::binding(11)]] Texture2D<float4> GBUFFER2 : register(t11);
[[vk::binding(12)]] Texture2D<float4> GBUFFER3 : register(t12);
SamplerState SAMPLER : register(s8);

void main(in PSInput i, out PSOutput o) {
  // HACK:locationの自動割り当てを誘導するため、指定したい番号順に読み出す
  const uint draw_id = G.draw_id;  // 0番

  // 深度を取り出す
  const float2 texcoord = i.position_ndc.xy * 0.5f + float2(0.5f, 0.5f);
  const float depth = DEPTH.Sample(SAMPLER, texcoord);

  // シェーディングしなくてもよいならば、早めに脱出する
  // TODO:なにもない所をスキップする、より正確な仕組みを導入する
  if (depth == 1) discard;

  // Gバッファから値を取り出す
  const float4 g0 = GBUFFER0.Sample(SAMPLER, texcoord);
  const float4 g1 = GBUFFER1.Sample(SAMPLER, texcoord);
  const float4 g2 = GBUFFER2.Sample(SAMPLER, texcoord);
  const float4 g3 = GBUFFER3.Sample(SAMPLER, texcoord);
  const float3 normal_w = decode_normal(g0.rgb);
  const float3 ambient = g1.rgb;
  const float3 diffuse = g2.rgb;
  const float3 specular = g3.rgb;
  const float specular_power = decode_specular_power(g3.a);

  // ワールド空間での位置を再構築する
  const float4 position_wh = mul(float4(i.position_ndc.xy, depth * 2.f - 1.f, 1.f), CAMERA.view_proj_inv);
  const float3 position_w = position_wh.xyz / position_wh.w;

  float3 final_color;
  switch (MODE) {
  case 0: {  // 通常
    const float3 v = normalize(CAMERA.position_w - position_w);  // 視線方向
    const float3 n = normalize(normal_w);  // 法線

    const PointLight light = LIGHTS[draw_id];

    const float3 lv = light.position_w - position_w;  // シェーディングポイントからライト位置へのベクトル
    const float l_len = length(lv);  // ライトまでの距離
    const float3 l = (light.position_w - position_w) / l_len;  // ライト方向
    const float3 r = reflect(-l, n);  // ライト方向の反射ベクトル

    // 減衰率を計算する
    // TODO:ちゃんとした減衰率を計算する
    float atten = 1.f;
    if (l_len >= light.radius) atten = 0.f;

    // Phongっぽく計算する
    final_color = 
        //ambient
        //+
        (diffuse * max(0.f, dot(n, l))
        +
        specular * pow(max(0.f, dot(v, r)), specular_power)
        ) * light.color * light.intensity * atten;
    break;
  }
  case 1: {  // 深度
      final_color = float3(depth, depth, depth);
    break;
  }
  case 2: {  // 法線
      final_color = g0.rgb;
    break;
  }
  case 3: {  // アンビエント
      final_color = g1.rgb;
    break;
  }
  case 4: {  // ディフューズ
      final_color = g2.rgb;
    break;
  }
  case 5: {  // スペキュラ
      final_color = g3.rgb;
    break;
  }
  case 6: {  // スペキュラパワー
      final_color = g3.aaa;
    break;
  }
  case 7: {  // 再構築された位置
      final_color = abs(position_w) % float3(1.f, 1.f, 1.f);
    break;
  }
  default: {
    final_color = float3(0.8f, 0.2f, 0.8f);
    break;
  }
  }

  o.frag_color = float4(final_color, 1.f);
}
