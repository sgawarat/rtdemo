/**
 * @brief Volumetric Fog - Pass 0: Voxelization
 */
#include "common.hlsli"

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);

// u
[[vk::binding(4)]] RWTexture3D<float4> u_vbuffer : register(u4);


[numthreads(8, 8, 1)]
void main(
  uint3 group_thread_id : SV_GroupThreadID,  // グループに対するスレッドの相対座標
  uint3 group_id : SV_GroupID,  // グループの絶対座標
  uint3 dispatch_thread_id : SV_DispatchThreadID,  // スレッドの絶対座標
  uint group_index : SV_GroupIndex  // グループ内で一意な番号
) {
  for (uint i = 0; i < 64; i++) {
    // froxelのIDからワールド空間の位置を計算する
    uint3 vid = uint3(dispatch_thread_id.xy, i);
    float3 texcoord_vol = float3(vid) / float3(FROXEL_COUNT);
    float3 position_vol = convert_from_texcoord_to_position(texcoord_vol);
    float3 position_v = convert_from_volume_to_view(position_vol, CAMERA);
    float3 position_w = mul(float4(position_v, 1.f), CAMERA.view_inv).xyz;

    // 形状に応じたボリュームの密度を測る
    float s = 0.f;
    const float scale = 3.f;
    switch (FOG_SHAPE) {
    case 0: {  // 高さ
      s = saturate(scale * (FOG_CENTER.y - position_w.y));
      break;
    }
    case 1: {  // 箱
      const float3 n = (FOG_CENTER - float1(FOG_RADIUS).xxx) - position_w;
      const float3 p =  position_w - (FOG_CENTER + float1(FOG_RADIUS).xxx);
      const float3 d = max(0.f, max(p, n));
      s = 1.f - saturate(scale * length(d));
      break;
    }
    case 2: {  // 球
      s = saturate(scale * (FOG_RADIUS - distance(position_w, FOG_CENTER)));
      break;
    }
    }

    u_vbuffer[vid] = s * float4(SCATTERING_COEFF, EXTINCTION_COEFF);
  }
}
