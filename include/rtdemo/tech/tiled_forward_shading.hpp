#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Tiled Forward Shading
 */
class TiledForwardShading final : public Technique {
 public:
  ~TiledForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(Scene& scene) override;

private:
  static constexpr size_t MAX_LIGHT_COUNT = 200;

  /**
   * @brief グリッドセル
   */
  struct GridCell {
    uint32_t first;
    uint32_t count;
  };

  /**
   * @brief デバッグ表示
   */
  enum class DebugView : int {
    DEFAULT,  ///< 通常
    POSITION,  ///< 位置
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
    TILE_INDEX,  ///< タイル番号
    TILE_LIGHT_COUNT,  ///< タイルのライト数
  };

  garie::Program p0_prog_;
  garie::Program p1_prog_;
  garie::Program p2_prog_;
  garie::Program p3_prog_;
  garie::Texture depth_tex_;
  garie::Texture rt0_tex_;
  garie::Framebuffer p0_fbo_;
  garie::Framebuffer p2_fbo_;
  garie::Viewport viewport_;
  garie::Buffer light_grid_ssbo_;
  garie::Buffer light_index_ssbo_;
  DebugView debug_view_ = DebugView::DEFAULT;
  size_t grid_width_ = 0;
  size_t grid_height_ = 0;
  std::string log_;  ///< シェーダのエラーログ
};
}  // namespace rtdemo::tech
