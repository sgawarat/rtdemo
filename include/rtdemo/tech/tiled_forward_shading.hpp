#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include "technique.hpp"

namespace rtdemo {
namespace tech {
class TiledForwardShading final : public Technique {
 public:
  ~TiledForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene& scene) override;

private:
  struct GridCell {
    uint32_t first;
    uint32_t count;
  };

  enum class DebugView : int {
    DEFAULT,
    POSITION,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    SPECULAR_POWER,
    TILE_INDEX,
    TILE_LIGHT_COUNT,
  };

  garie::Program p0_prog_;
  garie::Program p1_prog_;
  garie::Program p2_prog_;
  garie::Program p3_prog_;
  garie::Texture depth_tex_;
  garie::Texture rt0_tex_;
  garie::Framebuffer p0_fbo_;
  garie::Framebuffer p2_fbo_;
  garie::Buffer light_grid_ssbo_;
  garie::Buffer light_index_ssbo_;
  DebugView debug_view_ = DebugView::DEFAULT;
  GridCell cell_ = {};
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
