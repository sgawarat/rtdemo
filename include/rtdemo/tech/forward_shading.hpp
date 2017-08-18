#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include "technique.hpp"

namespace rtdemo {
namespace tech {
class ForwardShading final : public Technique {
 public:
  ~ForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene* scene) override;

 private:
  enum class DebugView : int {
    DEFAULT,
    POSITION,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    SPECULAR_POWER,
  };

  garie::Program prog_;
  DebugView debug_view_ = DebugView::DEFAULT;
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
