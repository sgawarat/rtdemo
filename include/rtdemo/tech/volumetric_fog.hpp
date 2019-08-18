#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Volumetric Fog
 */
class VolumetricFog final : public Technique {
 public:
  ~VolumetricFog() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(Scene& scene) override;

private:
  /**
   * @brief モード
   */
  enum class Mode : int {
    DEFAULT,  ///< 通常
    POSITION,  ///< 位置
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
    VTEXCOORD,  ///< Vバッファ用のテクスチャ座標
    SCATTERING,  ///< 散乱
    TRANSMITTANCE,  ///< 透過率
    VLIGHTING,  ///< ボリューメトリックライティング付き
  };

  /**
   * @brief フォグの形状
   */
  enum class FogShape : int {
    HIGHT,  // 高さ
    BOX,  // 箱
    SPHERE,  // 球
  };

  struct Constant {
    uint32_t froxel_count[3] = {};  // Vバッファの大きさ
    float _pad0;

    float volume_depth_scale = 1.f;  // ボリュームの深度にかけられる倍率
    float volume_depth_offset = 0.f;  // ボリュームの深度に加えられるオフセット
    float _pad1[2];

    float scattering_coeff[3] = {0.f};  // 散乱係数
    float extinction_coeff = 0.f;  // 消散係数

    Mode mode = Mode::DEFAULT;  // 表示モード
    FogShape fog_shape;  // フォグの形状
    float attenuation_coeff;  // 光の減衰に関する係数
    float _pad3;

    float fog_center[3];  // フォグの中心
    float fog_radius;  // フォグの半径
  };

  garie::Program p0_prog_;  // ボリュームのボクセル化
  garie::Program p1_prog_;  // ボリューメトリックライティングの計算
  garie::Program p2_prog_;  // レンダリング
  garie::Texture vbuffer_tex_;  // ボリュームの特性を格納する3Dテクスチャ
  garie::Texture lighting_tex_;  // 視点から見たのボリューメトリックライティングの結果を格納する3Dテクスチャ
  garie::Sampler lighting_ss_;  // 3Dテクスチャをサンプルするためのサンプラ
  garie::Buffer constant_ub_;  // 定数用バッファ
  Constant constant_;  // 定数の値
  std::string log_;  ///< シェーダのエラーログ
};
}  // namespace rtdemo::tech
