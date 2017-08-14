#pragma once

#include <cstdlib>

namespace rtdemo {
namespace scene {
class Scene {
 public:
  virtual ~Scene() noexcept {}
  virtual bool restore() = 0;
  virtual bool invalidate() = 0;
  virtual void update() = 0;
  virtual void update_gui() = 0;
  virtual void apply(size_t pass) = 0;
  virtual void draw(size_t pass) = 0;
};
}  // namespace scene
}  // namespace rtdemo
