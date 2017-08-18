#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include "garie.hpp"

namespace rtdemo {
namespace gui {
class Gui final {
 public:
  bool init(GLFWwindow* window, void(render_drawlists_fn)(ImDrawData*));

  void terminate();

  void new_frame();

  void render_drawlists(ImDrawData* draw_data);

  void on_mouse_button(int button, int action, int mods);
  void on_scroll(double x, double y);
  void on_key(int key, int scancode, int action, int mods);
  void on_char(unsigned int c);

 private:
  GLFWwindow* window_ = nullptr;
  double time_ = 0.0;
  bool mouse_pressed_[3] = {};
  float mouse_wheel_ = 0.f;
  garie::Program prog_;
  garie::RasterizationState rs_;
  garie::ColorBlendState bs_;
  garie::DepthStencilState dss_;
  garie::VertexArray vao_;
  garie::Buffer ibo_;
  garie::Buffer vbo_;
  garie::Texture font_tex_;
  garie::Sampler font_ss_;
};
}  // namespace gui
}  // namespace rtdemo
