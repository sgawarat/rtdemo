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
  };

  struct Constant {
    uint32_t froxel_count[3];
    Mode mode;
    float volume_depth_scale;
    float volume_depth_offset;
    float fog_height;
    float _pad[1];
  };

  garie::Program p0_prog_;  // ボリュームのボクセル化
  garie::Program p1_prog_;  // ボリューメトリックライティングの計算
  garie::Program p2_prog_;  // レンダリング
  garie::Texture vbuffer_tex_;
  garie::Texture lighting_tex_;
  garie::Sampler lighting_ss_;
  garie::Buffer constant_ub_;
  Mode mode_ = Mode::DEFAULT;
  uint32_t vbuffer_width_ = 0;
  uint32_t vbuffer_height_ = 0;
  uint32_t vbuffer_depth_ = 0;
  float volume_depth_scale_ = 1.f;
  float volume_depth_offset_ = 0.f;
  float fog_height_ = 1.f;
  std::string log_;  ///< シェーダのエラーログ
};
}  // namespace rtdemo::tech
