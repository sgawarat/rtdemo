#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/layout/static_layout.hpp>
#include <rtdemo/tech/technique.hpp>

namespace rtdemo {
namespace tech {
class DeferredShading : public Technique {
 public:
  using Layout = layout::StaticLayout;

  ~DeferredShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene* scene) override;

 private:
  garie::Program p0_prog_;
  garie::Program p1_prog_;
  garie::Texture ds_tex_;
  garie::Texture g0_tex_;
  garie::Texture g1_tex_;
  garie::Texture g2_tex_;
  garie::Texture g3_tex_;
  garie::Framebuffer fbo_;
  garie::Sampler ss_;
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
