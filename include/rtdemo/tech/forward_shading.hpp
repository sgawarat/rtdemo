#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/layout/static_layout.hpp>
#include <rtdemo/tech/technique.hpp>

namespace rtdemo {
namespace tech {
class ForwardShading : public Technique {
 public:
  using Layout = layout::StaticLayout;

  ~ForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene* scene) override;

 private:
  garie::Program prog_;
  std::string log_;
};
}  // namespace tech
}  // namespace rtdemo
