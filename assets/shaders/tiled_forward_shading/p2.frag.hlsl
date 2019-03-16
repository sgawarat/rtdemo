/**
 * @brief Tiled Forward Shading - Pass 2: Shading
 *
 * 前パスで生成されたライト番号のリストを用いて、通常のForward Shadingを行う
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

// 入力
struct PSInput {
  float4 position : SV_Position;
  [[vk::location(0)]] float3 position_w : POSITION_W;  // ワールド空間の位置
  [[vk::location(1)]] float3 normal_w : NORMAL_W;  // ワールド空間の法線
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
[[vk::binding(8)]] StructuredBuffer<Tile> TILES : register(t8);
[[vk::binding(9)]] StructuredBuffer<uint> LIGHT_INDICES : register(t9);
[[vk::binding(10)]] StructuredBuffer<uint> LIGHT_INDEX_COUNT : register(t10);

void main(in PSInput i, out PSOutput o) {
  // 描画に必要なリソースを取り出す
  const uint resource_index = RESOURCE_INDICES[G.draw_id];
  const Material material = MATERIALS[resource_index];

  // タイルを取り出す
  uint2 tile_id = uint2(i.position.xy) / uint2(TILE_WIDTH, TILE_HEIGHT);
  uint tile_index = tile_id.y * TILE_COUNT.x + tile_id.x;
  const Tile tile = TILES[tile_index];

  // シェーディングを行う
  float3 final_color = {0.f, 0.f, 0.f};
  switch (MODE) {
  case 0: {  // 通常
    float3 v = normalize(CAMERA.position_w - i.position_w);
    float3 n = normalize(i.normal_w);

    final_color = float3(0.f, 0.f, 0.f);//MATERIAL.ambient;
    for (uint k = 0; k < tile.light_index_count; k++) {
      const uint light_index = LIGHT_INDICES[tile.light_index_first + k];
      const PointLight light = LIGHTS[light_index];

      float3 lv = light.position_w - i.position_w;
      float l_len = length(lv);
      float atten = 1.f;
      if (l_len >= light.radius) atten = 0.f;
      float3 l = (light.position_w - i.position_w) / l_len;
      float3 r = reflect(-l, n);

      final_color +=
          (
              material.diffuse * max(0.f, dot(n, l))
              +
              material.specular * pow(max(0.f, dot(v, r)), material.specular_power)
          ) * light.color * light.intensity * atten;
    }
    break;
  }
  case 1: {  // 位置
    final_color = abs(i.position_w) % float1(1.f).xxx;
    break;
  }
  case 2: {  // 法線
    final_color = normalize(i.normal_w) * 0.5f + float1(0.5f).xxx;
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
  case 7: {  // タイルID
    final_color = float3(tile_id, 0.f) / float3(TILE_COUNT, 0.f);
    break;
  }
  case 8: {  // ライト数
    final_color = float1(tile.light_index_count).xxx / LIGHT_COUNT;
    break;
  }
  case 9: {  // シェーディングが行われたか
    float3 v = normalize(CAMERA.position_w - i.position_w);
    float3 n = normalize(i.normal_w);

    for (uint k = 0; k < tile.light_index_count; k++) {
      const uint light_index = LIGHT_INDICES[tile.light_index_first + k];
      const PointLight light = LIGHTS[light_index];

      if (length(light.position_w - i.position_w) >= light.radius) {
        final_color = float3(0.f, 0.8f, 0.f);
      } else {
        final_color = float3(0.8f, 0.f, 0.8f);
      }
    }
    break;
  }
  }

  o.frag_color = float4(final_color, 1.f);
}
