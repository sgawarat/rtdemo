#include "../common.hlsli"

// シーンの定数
cbuffer SceneConstant : register(b7) {
  uint LIGHT_COUNT;  // ライトの数
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
  uint3 FROXEL_COUNT;  // froxelの数
  float _pad0;

  float VOLUME_DEPTH_SCALE;  // ボリューム空間の深度にかける倍率
  float VOLUME_DEPTH_BIAS;  // ボリューム空間の深度に加えるオフセット
  float2 _pad1;

  float3 SCATTERING_COEFF;  // 散乱係数
  float EXTINCTION_COEFF;  // 消散係数

  int MODE;  // 表示するモード
  int FOG_SHAPE;  // フォグの形状
  float ATTEN_Q;  // 光の減衰を制御するパラメータ
  float _pad3;

  float3 FOG_CENTER;  // フォグの中心
  float FOG_RADIUS;  // フォグの半径

  float SHADOW_BIAS;  // シャドウ判定に加えるバイアス
  // float _pad5[3];
};

// ビュー空間のZ値からボリューム空間のZ値を計算する
// ニア面が0の近くに、ファー面が1の近くになるようにマッピングされる
// zは正の値であるとする
float encode_volume_depth(float z) {
  return log2(z - VOLUME_DEPTH_BIAS) / VOLUME_DEPTH_SCALE;
  // return z / VOLUME_DEPTH_SCALE;
}

// ボリューム空間のZ値からビュー空間のZ値を計算する
// 0がニア面の近くに、1がファー面の近くになるようにマッピングされる
// zは[0, 1]の中にあるとする
float decode_volume_depth(float z) {
  return pow(2.f, z * VOLUME_DEPTH_SCALE) + VOLUME_DEPTH_BIAS;
  // return z * VOLUME_DEPTH_SCALE;
}

// ボリューム空間の位置をビュー空間の位置に変換する
// position_volは[-1, 1]の中にあるとする
// 計算される位置は右手系のビュー空間にある
float3 convert_from_volume_to_view(float3 position_vol, Camera camera) {
  float z_view = -decode_volume_depth(position_vol.z * 0.5f + 0.5f);  // 右手系のために負にする
  float2 scale = 1.f / float2(camera.proj._11, camera.proj._22);
  return float3(position_vol.xy * z_view * scale, z_view);
}

// ビュー空間の位置をボリューム空間の位置に変換する
// position_vは右手系のビュー空間の位置である
// 計算される位置は[-1, 1]の中にある
float3 convert_from_view_to_volume(float3 position_v, Camera camera) {
  float z_vol = encode_volume_depth(-position_v.z) * 2.f - 1.f;
  float2 scale = {camera.proj._11, camera.proj._22};
  return float3(position_v.xy * scale / position_v.z, z_vol);
}

// [-1, 1]を[0, 1]に変換する
float3 convert_from_position_to_texcoord(float3 position) {
  return position * 0.5f + float1(0.5f).xxx;
}

// [0, 1]を[-1, 1]に変換する
float3 convert_from_texcoord_to_position(float3 texcoord) {
  return texcoord * 2.f - float1(1.f).xxx;
}

// Henyey-Greensteinの位相関数
// gは[-1, 1]の範囲で異方性を示すパラメータ
// cos_thetaは入射光と出射光のなす角のコサイン
float calc_hg_phase(float g, float cos_theta) {
  float g2 = g * g;
  float num = 1 - g2;
  float denom = pow(1 + g2 - 2 * g * cos_theta, 1.5f);
  return num / denom / M_4PI;
}

// 光の減衰率を計算する
float calc_attenuation(float dist) {
  // TODO:最大到達距離をベースにパラメータを決定する？
  return 1.f / max(1.f, (1.f / ATTEN_Q) * dist * dist);
}

// シグモイド関数
float sigmoid(float x) {
  return 1.f / (1.f + exp(-x));
}
