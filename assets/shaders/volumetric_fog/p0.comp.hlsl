/**
 * @brief Volumetric Fog - Pass 0: Voxelization
 */
#include <common.hlsli>
#include <volumetric_fog\\common.hlsli>

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
    uint3 vbuffer_coord = uint3(dispatch_thread_id.xy, i);
    u_vbuffer[vbuffer_coord] = float4(float3(vbuffer_coord) / float3(16.f, 16.f, 64.f), 0.f);
  }
}
