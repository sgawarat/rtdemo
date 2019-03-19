#include <rtdemo/tech/forward_shading.hpp>
#include <gsl/gsl>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(ForwardShading);

bool ForwardShading::restore() {
  // 成功しなければ、リソースを破棄するように設定する
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) invalidate();
  });

  // シェーダを読み込む
  garie::VertexShader vert = util::compile_vertex_shader_from_file(
      "forward_shading/p0.vert", &log_);
  if (!vert) return false;

  garie::FragmentShader frag =
  util::compile_fragment_shader_from_file(
          "forward_shading/p0.frag", &log_);
  if (!frag) return false;

  prog_ = util::link_program(vert, frag, &log_);
  if (!prog_) return false;

  // リソースを生成する
  constant_ub_.gen();
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, 1024, nullptr, GL_MAP_WRITE_BIT);

  log_ = "成功";

  succeeded = true;
  return true;
}

bool ForwardShading::invalidate() {
  prog_.del();
  constant_ub_.del();
  log_ = "利用不可";
  return true;
}

void ForwardShading::update() {
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  auto constant = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Mode), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (constant) {
    constant->mode = mode_;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }
}

void ForwardShading::update_gui() {
  ImGui::Begin("ForwardShading");
  ImGui::Combo("mode", reinterpret_cast<int*>(&mode_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0");
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

  // リソースをバインドする
  constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);

  // シーンを描画する
  scene.apply(ApplyType::SHADE);
  scene.draw(DrawType::OPAQUE);
}
}  // namespace rtrdemo::tech
