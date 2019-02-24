#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Deferred Shading
 * 
 */
class DeferredShading final : public Technique {
 public:
  ~DeferredShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(Scene& scene) override;

 private:
  /**
   * @brief デバッグ表示
   * 
   */
  enum class DebugView : int {
    DEFAULT,  ///< 通常
    DEPTH,  ///< 深度
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
    RECONSTRUCTED_POSITION,  ///< 再構築された位置
  };

  garie::Program p0_prog_;  ///< Gパスのプログラム
  garie::Program p1_prog_;  ///< Lパスのプログラム
  garie::Texture ds_tex_;
  garie::Texture g0_tex_;
  garie::Texture g1_tex_;
  garie::Texture g2_tex_;
  garie::Texture g3_tex_;
  garie::Framebuffer fbo_;
  garie::Viewport viewport_;
  garie::Sampler ss_;
  DebugView debug_view_ = DebugView::DEFAULT;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
