#include <rtdemo/tech/forward_shading.hpp>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(ForwardShading);

bool ForwardShading::restore() {
  // シェーダを読み込む
  garie::VertexShader vert = util::compile_vertex_shader_from_file(
      "forward_shading.vert", &log_);
  if (!vert) return false;

  garie::FragmentShader frag =
  util::compile_fragment_shader_from_file(
          "forward_shading.frag", &log_);
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
  // バックバッファをレンダターゲットにセットする
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  util::screen_viewport().apply();

  // バックバッファをクリアする
  util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);

  // パイプラインをバインドする
  prog_.use();
  util::default_rs().apply();
  util::alpha_blending_bs().apply();
  util::depth_test_dss().apply();

  // 定数をアップロードする
  glUniform1ui(32, static_cast<int>(debug_view_));

  // シーンを描画する
  scene.apply(ApplyType::SHADE);
  scene.draw(DrawType::OPAQUE);
}
}  // namespace rtrdemo::tech
