#include <rtdemo/tech/forward_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, ForwardShading);

namespace tech {
bool ForwardShading::restore() {
  garie::VertexShader vert = util::compile_shader_from_file<GL_VERTEX_SHADER>(
      "assets/shaders/forward_shading.vert", &log_);
  if (!vert) return false;

  garie::FragmentShader frag =
  util::compile_shader_from_file<GL_FRAGMENT_SHADER>(
          "assets/shaders/forward_shading.frag", &log_);
  if (!frag) return false;

  garie::Program prog = util::link_program(vert, frag, &log_);
  if (!prog) return false;

  // finalize
  prog_ = std::move(prog);
  log_ = "succeeded";
  return true;
}

bool ForwardShading::invalidate() {
  prog_ = garie::Program();
  log_ = "not available";
  return true;
}

void ForwardShading::update() {}

void ForwardShading::update_gui() {
  ImGui::Begin("ForwardShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void ForwardShading::apply(scene::Scene* scene) {
  if (prog_) prog_.use();
  glUniform1ui(11, static_cast<int>(debug_view_));  
  util::default_rs().apply();
  util::alpha_blending_bs().apply();
  util::depth_test_dss().apply();
  if (scene) scene->draw(scene::PassType::SHADE);
}
}  // namespace tech
}  // namespace rtrdemo
