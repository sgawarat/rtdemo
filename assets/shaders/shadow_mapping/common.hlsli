/**
 * Shadow Mapping
 */

// シーンの定数
cbuffer SceneConstant : register(b7) {
  uint SHADOW_CASTER_COUNT;  // シャドウキャスタの数
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
  uint SHADOW_CASTER_INDEX;  // 表示するシャドウキャスタ番号
  int MODE;  // 表示するモード
};

// shadow acneを回避するための深度の調整値
#define SHADOW_BIAS 0.00005f
