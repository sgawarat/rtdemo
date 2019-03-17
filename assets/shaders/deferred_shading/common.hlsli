// 法線をGバッファに格納できるようにエンコードする
float3 encode_normal(float3 normal) {
  return normal * 0.5f + float3(0.5f, 0.5f, 0.5f);
}

// Gバッファに格納された値から法線をデコードする
float3 decode_normal(float3 g) {
  return normalize(g * 2.f - float3(1.f, 1.f, 1.f));
}

// スペキュラパワーをGバッファに格納できるようにエンコードする
float encode_specular_power(float specular_power) {
  return log2(specular_power) / 10.5f;
}

// Gバッファに格納された値からスペキュラパワーをデコードする
float decode_specular_power(float g) {
  return exp2(g * 10.5f);
}

// シーンの定数
cbuffer SceneConstant : register(b7) {
  int _unused;
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
    int MODE;  // 表示するモード
};
