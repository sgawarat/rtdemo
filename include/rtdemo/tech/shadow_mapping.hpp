#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Shadow Mapping
 */
class ShadowMapping final : public Technique {
 public:
  ~ShadowMapping() noexcept override {}

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
    SHADOWED,  ///< 影付けされた所を色付けする
  };

  struct Constant {
    uint32_t shadow_caster_index = 0;
    Mode mode = Mode::DEFAULT;
    float _pad[2];
  };

  garie::Program p0_prog_;  ///< シャドウパスのプログラム
  garie::Program p1_prog_;  ///< シェーディングパスのプログラム
  garie::Buffer constant_ub_;
  garie::Texture depth_tex_;  ///< シャドウマップのテクスチャ
  garie::Framebuffer p0_fbo_;  ///< シャドウパスのフレームバッファ
  garie::Viewport p0_viewport_;  ///< シャドウパスのビューポート
  garie::Sampler ss_;  ///< サンプラ
  Constant constant_;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
