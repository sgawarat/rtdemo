#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Deferred Shading
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
   * @brief モード
   */
  enum class Mode : int {
    DEFAULT,  ///< 通常
    DEPTH,  ///< 深度
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
    RECONSTRUCTED_POSITION,  ///< 再構築された位置
  };

  struct Constant {
    Mode mode;
    float _pad[3];
  };

  garie::Program p0_prog_;  ///< Gパスのプログラム
  garie::Program p1_prog_;  ///< Lパスのプログラム
  garie::Buffer constant_ub_;  ///< 定数用バッファ
  garie::Texture ds_tex_;  ///< 深度ステンシル
  garie::Texture g0_tex_;  ///< Gバッファ（1枚目）
  garie::Texture g1_tex_;  ///< Gバッファ（2枚目）
  garie::Texture g2_tex_;  ///< Gバッファ（3枚目）
  garie::Texture g3_tex_;  ///< Gバッファ（4枚目）
  garie::Framebuffer fb_;
  garie::Viewport viewport_;
  garie::Sampler ss_;
  Mode mode_ = Mode::DEFAULT;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
