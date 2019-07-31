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

    // 高さフォグっぽいことをする
    float3 s = float1(saturate(FOG_HEIGHT - position_w.y)).xxx;
    u_vbuffer[vid] = float4(s * SCATTERING_COEFF, EXTINCTION_COEFF);
  }
}
