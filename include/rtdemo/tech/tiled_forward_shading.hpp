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
  static constexpr size_t TILE_WIDTH = 32;
  static constexpr size_t TILE_HEIGHT = 32;
  static constexpr size_t MAX_LIGHT_COUNT = 200;

  /**
   * @brief タイル
   */
  struct Tile {
    uint32_t light_index_first;
    uint32_t light_index_count;
  };

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
    TILE_INDEX,  ///< タイル番号
    TILE_LIGHT_COUNT,  ///< タイルのライト数
    SHADED,  ///< シェーディングされたか
  };

  struct Constant {
    uint32_t tile_count[2];
    uint32_t pixel_count[2];
    Mode mode;
    float _pad[3];
  };

  struct Print {
    float light_position_v[4];
    float frustum_plane0[4];
    float dot_product;
    float light_radius;
    float _pad[2];
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
  garie::Buffer constant_ubo_;
  garie::Buffer tiles_ssbo_;
  garie::Buffer light_indices_ssbo_;
  garie::Buffer light_index_count_ssbo_;
  garie::Buffer print_ssbo_;
  Mode mode_ = Mode::DEFAULT;
  uint32_t tiled_screen_width_ = 0;
  uint32_t tiled_screen_height_ = 0;
  std::string log_;  ///< シェーダのエラーログ
};
}  // namespace rtdemo::tech
