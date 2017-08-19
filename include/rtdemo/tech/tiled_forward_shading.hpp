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
    float _0z;
    float _0w;
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
  };

  garie::Program p0_prog_;
  garie::Program p1_prog_;
  garie::Program p2_prog_;
  garie::Buffer light_grid_ssbo_;
  garie::Buffer light_index_ssbo_;
  DebugView debug_view_ = DebugView::DEFAULT;
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
