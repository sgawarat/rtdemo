/**
 * @brief Volumetric Fog - Pass 1: Volumetric Lighting
 */
#include "common.hlsli"

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);
[[vk::binding(1)]] StructuredBuffer<ShadowCaster> SHADOW_CASTERS : register(t1);
[[vk::binding(3)]] Texture2D<float> SHADOW : register(t3);

// s
[[vk::binding(3)]] SamplerState SHADOW_SAMPLER : register(s3);

// u
[[vk::binding(4)]] RWTexture3D<float4> VBUFFER : register(u4);  // TODO:読み込み専用にする
[[vk::binding(5)]] RWTexture3D<float4> u_lighting : register(u5);

float3 calc_froxel_scattering(float3 position_w, float3 phase, float3 light_radiance, float sigma_t, float depth, float4x4 shadow_matrix) {
  float visibility = 1.f;
  if (true) {  // TODO:シャドウを持たないライトに対応する
    const float4 position_sm = mul(float4(position_w, 1.f), shadow_matrix);
    const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
    const float shadow_depth = SHADOW.SampleLevel(SHADOW_SAMPLER, shadow_coord.xy, 0.f);
    if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
      visibility = 0.f;
    } else {
      visibility = 1.f;
    }
  }
  return phase * visibility * light_radiance * (1.f - exp(-sigma_t * depth)) / (sigma_t + M_EPSILON);
}

// VBUFFERに格納されたボリュームの特性をもとに、froxel内に発生する散乱を計算する。
// u_lightingには視点からそのfroxelまでを見たときの累積的な散乱の結果を格納する。
// 積分を単純にするため、froxel内は均質であると仮定している。
[numthreads(8, 8, 1)]
void main(
  uint3 group_thread_id : SV_GroupThreadID,  // グループに対するスレッドの相対座標
  uint3 group_id : SV_GroupID,  // グループの絶対座標
  uint3 dispatch_thread_id : SV_DispatchThreadID,  // スレッドの絶対座標
  uint group_index : SV_GroupIndex  // グループ内で一意な番号
) {
  // 最前面の位置を計算する
  const uint3 most_front_vid = uint3(dispatch_thread_id.xy, 0);
  const float3 most_front_texcoord_vol = float3(most_front_vid) / float3(FROXEL_COUNT);
  const float3 most_front_position_vol = convert_from_texcoord_to_position(most_front_texcoord_vol);
  const float3 most_front_position_v = convert_from_volume_to_view(most_front_position_vol, CAMERA);
  const float4 most_front_position_wh = mul(float4(most_front_position_v, 1.f), CAMERA.view_inv);
  const float3 most_front_position_w = most_front_position_wh.xyz / most_front_position_wh.w;

  // 各froxelまでの散乱と透過率を計算する
  float3 total_scattering = 0.f;  // 散乱の総和
  float total_transmittance = 1.f;  // 透過率の総乗
  float3 prev_front_position_w = most_front_position_w;
  for (uint i = 0; i < 64; ++i) {
    // froxel内で一定の値を用意する
    const uint3 front_vid = uint3(dispatch_thread_id.xy, i);
    const float4 property = VBUFFER.Load(front_vid);
    const float3 sigma_s = property.rgb;
    const float sigma_t = property.a;

    // subfroxel内の散乱光を計算する
    float3 froxel_scattering = 0.f;
    float3 prev_back_position_w = prev_front_position_w;
    const uint subfroxel_count = 4;
    for (uint j = 1; j <= subfroxel_count; ++j) {
      // subfroxel内で一定の値を用意する
      const float3 back_texcoord_vol = (float3(front_vid) + float3(0.f, 0.f, j / float(subfroxel_count))) / float3(FROXEL_COUNT);
      const float3 back_position_vol = convert_from_texcoord_to_position(back_texcoord_vol);
      const float3 back_position_v = convert_from_volume_to_view(back_position_vol, CAMERA);
      const float4 back_position_wh = mul(float4(back_position_v, 1.f), CAMERA.view_inv);
      const float3 back_position_w = back_position_wh.xyz / back_position_wh.w;
      const float subfroxel_depth = distance(back_position_w, prev_back_position_w);
      const float3 view_w = normalize(CAMERA.position_w - back_position_w);

      // 各光源に対する散乱光を計算する
      for (uint light_index = 0; light_index < 1; light_index++) {
        const PointLight light = LIGHTS[light_index];
        const float3 light_w = normalize(light.position_w);
        const float v_l = dot(view_w, light_w);
        const float phase = calc_hg_phase(0.f, v_l);
        const float3 light_radiance = light.color * light.intensity;
        froxel_scattering += calc_froxel_scattering(back_position_w, phase, light_radiance, sigma_t, subfroxel_depth, SHADOW_CASTERS[light_index].view_proj);
      }

      // ポイントライトに対する散乱光を計算する
      for (; light_index < LIGHT_COUNT; light_index++) {
        const PointLight light = LIGHTS[light_index];
        const float light_distance = distance(light.position_w, back_position_w);
        const float3 light_w = normalize(light.position_w - back_position_w);
        const float v_l = dot(view_w, light_w);
        const float phase = calc_hg_phase(0.f, v_l);
        const float3 light_radiance = light.color * light.intensity;
        const float atten = calc_attenuation(light_distance);
        froxel_scattering += calc_froxel_scattering(back_position_w, phase, light_radiance, sigma_t, subfroxel_depth, SHADOW_CASTERS[light_index].view_proj) * atten;
      }

      prev_back_position_w = back_position_w;
    }

    // 括りだした値をかける
    froxel_scattering *= total_transmittance * sigma_s;

    // froxel内の透過率を計算する
    const float froxel_depth = distance(prev_front_position_w, prev_back_position_w);
    const float froxel_transmittance = exp(-sigma_t * froxel_depth);

    // 視点からの累計値を計算する
    total_transmittance *= froxel_transmittance;
    total_scattering += froxel_scattering;
    u_lighting[front_vid] = float4(total_scattering, total_transmittance);

    prev_front_position_w = prev_back_position_w;
  }
}
