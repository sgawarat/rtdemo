/**
 * @brief Tiled Forward Shading - Pass 1: Light Assignment
 */
#include <common.hlsli>
#include <tiled_forward_shading\\common.hlsli>

// 入力
struct CSInput {
  uint3 group_thread_id : SV_GroupThreadID;  // グループに対するスレッドの相対座標
  uint3 group_id : SV_GroupID;  // グループの絶対座標
  uint3 dispatch_thread_id : SV_DispatchThreadID;  // スレッドの絶対座標
  uint group_index : SV_GroupIndex;  // グループ内で一意な番号
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);
[[vk::binding(8)]] Texture2D<float> DEPTH : register(t8);

// u
[[vk::binding(8)]] RWStructuredBuffer<Tile> u_tiles : register(u8);
[[vk::binding(9)]] RWStructuredBuffer<uint> u_light_indices : register(u9);
[[vk::binding(10)]] RWStructuredBuffer<uint> u_light_index_count : register(u10);

struct Print {
  float4 light_position_v;
  float4 frustum_plane0;
  float dot_product;
  float light_radius;
};
[[vk::binding(15)]] RWStructuredBuffer<Print> u_print : register(u15);

// グループで共有する値
groupshared uint s_min_depth_uint;  // 深度の最小値(の整数表現)
groupshared uint s_max_depth_uint;  // 深度の最大値(の整数表現)
groupshared uint s_light_indices[MAX_LIGHT_INDEX_COUNT];  // ライト番号リスト
groupshared uint s_light_index_first;  // ライト番号のオフセット
groupshared uint s_light_index_count;  // ライト番号の数
groupshared TileFrustum s_tile_frustum;  // タイルの視錐台

// タイル座標をクリップ空間での位置に変換する
float4 to_clip(uint2 tile_id) {
  float2 position_c = float2(tile_id) / float2(TILE_COUNT) * 2.f - float2(1.f, 1.f);
  return float4(position_c, 1.f, 1.f);
}

// タイルの視錐台を計算する
TileFrustum new_tile_frustum(uint2 tile_id) {
  // TODO:DXとGLの座標系の違いを考慮する
  float4 lt = mul(to_clip(tile_id + uint2(0, 1)), CAMERA.proj_inv);
  float4 rt = mul(to_clip(tile_id + uint2(1, 1)), CAMERA.proj_inv);
  float4 lb = mul(to_clip(tile_id + uint2(0, 0)), CAMERA.proj_inv);
  float4 rb = mul(to_clip(tile_id + uint2(1, 0)), CAMERA.proj_inv);
  lt /= lt.w;
  rt /= rt.w;
  lb /= lb.w;
  rb /= rb.w;

  TileFrustum frustum;
  frustum.planes[0] = float4(normalize(cross(lt.xyz, lb.xyz)), 0.f);  // 右
  frustum.planes[1] = float4(normalize(cross(rb.xyz, rt.xyz)), 0.f);  // 左
  frustum.planes[2] = float4(normalize(cross(rt.xyz, lt.xyz)), 0.f);  // 下
  frustum.planes[3] = float4(normalize(cross(lb.xyz, rb.xyz)), 0.f);  // 上
  return frustum;
}

[numthreads(TILE_WIDTH, TILE_HEIGHT, 1)]
void main(CSInput i) {
  float depth;
  if (i.dispatch_thread_id.x < PIXEL_COUNT.x &&
      i.dispatch_thread_id.y < PIXEL_COUNT.y) {
    // スクリーン内の深度を取得する
    depth = DEPTH.Load(int3(i.dispatch_thread_id.xy, 0));
  } else {
    // スクリーン外の深度は最奥とする
    depth = 1.f;
  }

  // HACK:Interlocked{Min,Max}は整数型しかないので、float型をいったんuint型に再解釈してから行う必要がある
  uint depth_uint = asuint(depth);

  // 各グループの0番スレッドがgroupshared変数を初期化する
  if (i.group_index == 0) {
    s_min_depth_uint = 0x3f800000;  // = 1.f
    s_max_depth_uint = 0;
    s_light_index_count = 0;
    s_tile_frustum = new_tile_frustum(i.group_id.xy);

    // 単一のスレッドがライト番号リストのリセットを行う
    if (i.dispatch_thread_id.x == 0 &&
        i.dispatch_thread_id.y == 0) {
      u_light_index_count[0] = 0;
    }
  }

  GroupMemoryBarrierWithGroupSync();  // groupshared変数の初期化が完了するのを待つ

  // タイル内の深度の最大最小値を求める
  InterlockedMin(s_min_depth_uint, depth_uint);
  InterlockedMax(s_max_depth_uint, depth_uint);

  GroupMemoryBarrierWithGroupSync();  // 深度の最大最小値の計算が完了するのを待つ

  // ビュー空間での深度の最大最小値を求める
  float min_depth = asfloat(s_min_depth_uint);
  float max_depth = asfloat(s_max_depth_uint);
  float4 near_vh = mul(float4(0.f, 0.f, min_depth * 2.f - 1.f, 1.f), CAMERA.proj_inv);
  float4 far_vh = mul(float4(0.f, 0.f, max_depth * 2.f - 1.f, 1.f), CAMERA.proj_inv);
  float near_v = near_vh.z / near_vh.w;
  float far_v = far_vh.z / far_vh.w;

  // ライトをカリングする
  for (uint k = i.group_index; k < LIGHT_COUNT; k += TILE_SIZE) {
    PointLight light = LIGHTS[k];
    float4 light_position_v = mul(float4(light.position_w, 1.f), CAMERA.view);

    if (dot(light_position_v, s_tile_frustum.planes[0]) < light.radius &&
        dot(light_position_v, s_tile_frustum.planes[1]) < light.radius &&
        dot(light_position_v, s_tile_frustum.planes[2]) < light.radius &&
        dot(light_position_v, s_tile_frustum.planes[3]) < light.radius &&
        light_position_v.z - light.radius <= near_v &&
        light_position_v.z + light.radius >= far_v &&
        true
    ) {
      // ライトがそのタイルで可視であるならば、ライト番号リストに追加する
      uint offset;
      InterlockedAdd(s_light_index_count, 1, offset);
      if (offset < MAX_LIGHT_INDEX_COUNT) {
        s_light_indices[offset] = k;
      }
    }
  }

  GroupMemoryBarrierWithGroupSync();  // カリングが完了するのを待つ


  // 各グループの0番スレッドがライト番号リストの領域の確保を行う
  if (i.group_index == 0) {
    InterlockedAdd(u_light_index_count[0], s_light_index_count, s_light_index_first);

    const Tile tile = {
      s_light_index_first,
      s_light_index_count,
    };
    uint tile_index = i.group_id.y * TILE_COUNT.x + i.group_id.x;
    u_tiles[tile_index] = tile;
  }

  GroupMemoryBarrierWithGroupSync();  // 領域の確保が完了するのを待つ

  // ライト番号をコピーする
  for (uint k2 = i.group_index; k2 < s_light_index_count; k2 += TILE_SIZE) {
    u_light_indices[s_light_index_first + k2] = s_light_indices[k2];
  }

  // デバッグ用に中間値をコピーする
  if (i.group_index == 0) {
    PointLight light = LIGHTS[0];
    float4 light_position_v = mul(float4(light.position_w, 1.f), CAMERA.view);

    Print print;
    print.light_position_v = light_position_v;
    print.frustum_plane0 = s_tile_frustum.planes[0];
    print.dot_product = dot(light_position_v, s_tile_frustum.planes[0]);
    print.light_radius = light.radius;

    uint tile_index = i.group_id.y * TILE_COUNT.x + i.group_id.x;
    u_print[tile_index] = print;
  }
  
}
