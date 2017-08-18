#pragma once

#include <vector>
#include "../garie.hpp"
#include "scene.hpp"

namespace rtdemo {
namespace scene {
class StaticScene final : public Scene {
 public:
  ~StaticScene() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void draw(PassType type) override;

 private:
  enum class DrawMode : int {
    DRAW,
    DRAW_INDIRECT,
  };

  struct Command {
    GLuint index_count;
    GLuint instance_count;
    GLuint index_first;
    GLuint base_vertex;
    GLuint base_instance;
  };

  float camera_center_ = 0.f;
  float camera_distance_ = 0.f;
  float camera_yaw_ = 0.f;
  float camera_pitch_ = 0.f;
  float lens_depth_ = 100.f;
  DrawMode draw_mode_ = DrawMode::DRAW;

  garie::VertexArray vao_;
  garie::Buffer vbo_;
  garie::Buffer ibo_;
  garie::Buffer camera_ubo_;
  garie::Buffer resource_index_ssbo_;
  garie::Buffer material_ssbo_;
  garie::Buffer light_ssbo_;
  size_t light_count_ = 0;
  garie::Buffer dio_;
  std::vector<Command> commands_;
};
}  // namespace scene
}  // namespace rtdemo
