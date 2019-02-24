#include <rtdemo/tech/forward_shading.hpp>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(ForwardShading);

bool ForwardShading::restore() {
  // シェーダを読み込む
  garie::VertexShader vert = util::compile_vertex_shader_from_file(
      "assets/shaders/forward_shading.vert", &log_);
  if (!vert) return false;

  garie::FragmentShader frag =
  util::compile_fragment_shader_from_file(
          "assets/shaders/forward_shading.frag", &log_);
  if (!frag) return false;

  garie::Program prog = util::link_program(vert, frag, &log_);
  if (!prog) return false;

  // 後始末
  prog_ = std::move(prog);
  log_ = "成功";
  return true;
}

bool ForwardShading::invalidate() {
  prog_ = garie::Program();
  log_ = "利用不可";
  return true;
}

void ForwardShading::update() {}

void ForwardShading::update_gui() {
  ImGui::Begin("ForwardShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void ForwardShading::apply(Scene& scene) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glViewport(0, 0, 1280, 720);
  glScissor(0, 0, 1280, 720);

  // バックバッファをクリアする
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClearDepthf(1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  prog_.use();
  glUniform1ui(11, static_cast<int>(debug_view_));  
  util::default_rs().apply();
  util::alpha_blending_bs().apply();
  util::depth_test_dss().apply();

  // 定数をアップロードする
  scene.apply(ApplyType::SHADE);
  scene.draw(DrawType::OPAQUE);
}
}  // namespace rtrdemo::tech
