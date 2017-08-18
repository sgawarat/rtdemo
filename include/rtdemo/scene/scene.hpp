#pragma once

namespace rtdemo {
namespace scene {
enum class PassType {
  SHADE,
  NO_SHADE,
  LIGHT,
};

 class Scene {
 public:
  virtual ~Scene() noexcept {}
  virtual bool restore() = 0;
  virtual bool invalidate() = 0;
  virtual void update() = 0;
  virtual void update_gui() = 0;
  virtual void draw(PassType type) = 0;
};
}  // namespace scene
}  // namespace rtdemo
