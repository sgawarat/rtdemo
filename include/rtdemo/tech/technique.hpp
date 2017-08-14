#pragma once

#include <rtdemo/scene/scene.hpp>

namespace rtdemo {
namespace tech {
class Technique {
 public:
  virtual ~Technique() noexcept {}
  virtual bool restore() = 0;
  virtual bool invalidate() = 0;
  virtual void update() = 0;
  virtual void update_gui() = 0;
  virtual void apply(scene::Scene* scene) = 0;
};
}  // namespace tech
}  // namespace rtdemo
