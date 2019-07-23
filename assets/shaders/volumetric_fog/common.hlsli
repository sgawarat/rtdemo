// シーンの定数
cbuffer SceneConstant : register(b7) {
  uint LIGHT_COUNT;  // ライトの数
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
  int MODE;  // 表示するモード
};
