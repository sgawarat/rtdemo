#pragma once

namespace rtdemo {
namespace scene {
enum class ApplyType {
  SHADE,
  NO_SHADE,
  LIGHT,
};

enum class DrawType {
  OPAQUE,
  TRANSPARENT,
  LIGHT_VOLUME,
};

class Scene {
 public:
  virtual ~Scene() noexcept {}
  virtual bool restore() = 0;
  virtual bool invalidate() = 0;
  virtual void update() = 0;
  virtual void update_gui() = 0;
  virtual void apply(ApplyType type) = 0;
  virtual void draw(DrawType type) = 0;
};
}  // namespace scene
}  // namespace rtdemo
