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
  uint3 front_vid = uint3(dispatch_thread_id.xy, 0);
  float3 front_texcoord_vol = float3(front_vid) / float3(FROXEL_COUNT);
  float3 front_position_vol = convert_from_texcoord_to_position(front_texcoord_vol);
  float3 front_position_v = convert_from_volume_to_view(front_position_vol, CAMERA);

  // 各froxelまでの散乱と透過率を計算する
  float3 total_scattering = 0.f;  // 散乱の総和
  float total_transmittance = 1.f;  // 透過率の総乗
  for (uint i = 1; i <= 64; i++) {
    // froxelの背面の位置を計算する
    const uint3 back_vid = uint3(dispatch_thread_id.xy, i);
    const float3 back_texcoord_vol = float3(back_vid) / float3(FROXEL_COUNT);
    const float3 back_position_vol = convert_from_texcoord_to_position(back_texcoord_vol);
    const float3 back_position_v = convert_from_volume_to_view(back_position_vol, CAMERA);
    const float4 back_position_wh = mul(float4(back_position_v, 1.f), CAMERA.view_inv);
    const float3 back_position_w = back_position_wh.xyz / back_position_wh.w;

    // 必要な値を計算する
    const float froxel_depth = abs(back_position_v.z - front_position_v.z);
    const float3 view_v = normalize(-back_position_v);

    // 媒質の特性を取り出す
    const float4 property = VBUFFER.Load(front_vid);
    const float3 sigma_s = property.rgb;
    const float sigma_t = property.a;
    
    // froxel内の透過率を計算する
    float froxel_transmittance = exp(-sigma_t * froxel_depth);

    // froxel内の散乱光を計算する
    float3 froxel_in_scattering = 0.f;

    // ディレクショナルライト
    for (uint light_index = 0; light_index < 1; light_index++) {
      const PointLight light = LIGHTS[light_index];
      const float3 light_position_v = mul(float4(light.position_w, 1.f), CAMERA.view).xyz;

      // 遮蔽を計算する
      // TODO:最適なサンプリング方法を考える
      float visibility = 1.f;
      if (light_index == 0) {  // TODO:シャドウを持たないライトに対応する
        const float4 position_sm = mul(float4(back_position_w, 1.f), SHADOW_CASTERS[light_index].view_proj);
        const float3 shadow_coord = (position_sm.xyz / position_sm.w) * float1(0.5f).xxx + float1(0.5f).xxx;
        const float shadow_depth = SHADOW.SampleLevel(SHADOW_SAMPLER, shadow_coord.xy, 0.f);
        if (shadow_depth < (shadow_coord.z - SHADOW_BIAS)) {
          visibility = 0.f;
        } else {
          visibility = 1.f;
        }
      }

      // ライティングを計算する
      const float light_distance = length(light_position_v);
      const float3 light_v = normalize(light_position_v);
      const float v_l = dot(view_v, light_v);
      const float phase = calc_hg_phase(0.f, v_l);
      const float atten = 1.f;//calc_attenuation(light_distance);
      froxel_in_scattering += phase * light.color * light.intensity * atten * visibility;
    }

    // ポイントライト
    for (; light_index < LIGHT_COUNT; light_index++) {
      const PointLight light = LIGHTS[light_index];
      const float3 light_position_v = mul(float4(light.position_w, 1.f), CAMERA.view).xyz;

      // ライティングを計算する
      const float light_distance = length(light_position_v - back_position_v);
      const float3 light_v = normalize(light_position_v - back_position_v);
      const float v_l = dot(view_v, light_v);
      const float phase = calc_hg_phase(0.f, v_l);
      const float atten = calc_attenuation(light_distance);
      froxel_in_scattering += phase * light.color * light.intensity * atten;
    }

    float3 froxel_scattering = total_transmittance * sigma_s * froxel_in_scattering * (1.f - froxel_transmittance) / (sigma_t + M_EPSILON);

    // 視点からの累計値を計算する
    total_transmittance *= froxel_transmittance;
    total_scattering += froxel_scattering;
    u_lighting[front_vid] = float4(total_scattering, total_transmittance);

    // 次の反復のために値をコピーする
    front_vid = back_vid;
    front_texcoord_vol = back_texcoord_vol;
    front_position_vol = back_position_vol;
    front_position_v = back_position_v;
  }
}
