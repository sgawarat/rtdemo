#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include "technique.hpp"

namespace rtdemo {
namespace tech {
class DeferredShading final : public Technique {
 public:
  ~DeferredShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene& scene) override;

 private:
  enum class DebugView : int {
    DEFAULT,
    DEPTH,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    SPECULAR_POWER,
    RECONSTRUCTED_POSITION,
  };

  garie::Program p0_prog_;
  garie::Program p1_prog_;
  garie::Texture ds_tex_;
  garie::Texture g0_tex_;
  garie::Texture g1_tex_;
  garie::Texture g2_tex_;
  garie::Texture g3_tex_;
  garie::Framebuffer fbo_;
  garie::Sampler ss_;
  DebugView debug_view_ = DebugView::DEFAULT;
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
