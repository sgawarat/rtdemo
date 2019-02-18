#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include "technique.hpp"

namespace rtdemo::tech {
/**
 * @brief Shadow Mapping
 * 
 * 
 */
class ShadowMapping final : public Technique {
 public:
  ~ShadowMapping() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene& scene) override;

 private:
  /**
   * @brief デバッグ表示
   * 
   */
  enum class DebugView : int {
    DEFAULT,  ///< 通常
    SHADOWED,  ///< 影付けされた所を色付けする
    CASTER,  ///< シャドウキャスタの視点
  };

  garie::Program p0_prog_;  ///< シャドウパスのプログラム
  garie::Program p1_prog_;  ///< シェーディングパスのプログラム
  garie::Texture depth_tex_;  ///< シャドウマップのテクスチャ
  garie::Framebuffer p0_fbo_;  ///< シャドウパスのフレームバッファ
  garie::Sampler ss_;  ///< サンプラ
  DebugView debug_view_ = DebugView::DEFAULT;
  float shadow_bias_ = 0.05f;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
