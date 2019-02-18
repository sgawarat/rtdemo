#include <rtdemo/tech/shadow_mapping.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
namespace {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, ShadowMapping);
}  // namespace

bool ShadowMapping::restore() {
  // シェーダを読み込む
  garie::VertexShader p0_vert = util::compile_vertex_shader_from_file(
      "assets/shaders/shadow_mapping_p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert = util::compile_vertex_shader_from_file(
      "assets/shaders/shadow_mapping_p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag = util::compile_fragment_shader_from_file(
      "assets/shaders/shadow_mapping_p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag = util::compile_fragment_shader_from_file(
      "assets/shaders/shadow_mapping_p1.frag", &log_);
  if (!p1_frag) return false;

  garie::Program p0_prog = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog) return false;

  garie::Program p1_prog = util::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog) return false;

  // リソースを生成する
  garie::Texture depth_tex;
  depth_tex.gen();
  depth_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 1024, 1024);

  garie::Framebuffer p0_fbo =
      garie::FramebufferBuilder().depth_texture(depth_tex).build();

  const float border_color[4] = {1.f, 1.f, 1.f, 1.f};
  garie::Sampler ss = garie::SamplerBuilder()
                          .min_filter(GL_NEAREST)
                          .mag_filter(GL_NEAREST)
                          .wrap_s(GL_CLAMP_TO_BORDER)
                          .wrap_t(GL_CLAMP_TO_BORDER)
                          .border_color(border_color)
                          .build();

  // 後始末
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  depth_tex_ = std::move(depth_tex);
  p0_fbo_ = std::move(p0_fbo);
  ss_ = std::move(ss);
  log_ = "成功";
  return true;
}

bool ShadowMapping::invalidate() {
  ss_ = {};
  p0_fbo_ = {};
  depth_tex_ = {};
  p0_prog_ = {};
  p1_prog_ = {};
  log_ = "利用不可";
  return true;
}

void ShadowMapping::update() {}

void ShadowMapping::update_gui() {
  ImGui::Begin("ShadowMapping");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_),
               "Default\0SHADOWED\0CASTER\0");
  ImGui::DragFloat("Bias * 100", &shadow_bias_, 0.01f, -1.f, 1.f);
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void ShadowMapping::apply(scene::Scene& scene) {
  // パス0:シャドウ
  {
    // 深度バッファのみのFBOをバインドする
    p0_fbo_.bind(GL_DRAW_FRAMEBUFFER);

    glViewport(0, 0, 1024, 1024);
    glScissor(0, 0, 1024, 1024);

    // 深度バッファをクリアする
    glDepthMask(GL_TRUE);
    glClearDepthf(1.f);
    glClear(GL_DEPTH_BUFFER_BIT);

    //  ステートをバインドする
    p0_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // シーンを描画する
    scene.apply(scene::ApplyType::SHADOW);
    scene.draw(scene::DrawType::OPAQUE);
  }

  // パス1:シェーディング
  {
    // バックバッファをターゲットにする
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glViewport(0, 0, 1280, 720);
    glScissor(0, 0, 1280, 720);

    // バックバッファをクリアする
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClearDepthf(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ステートをバインドする
    p1_prog_.use();
    depth_tex_.active(0, GL_TEXTURE_2D);
    ss_.bind(0);
    util::default_rs().apply();
    util::alpha_blending_bs().apply();
    util::depth_test_dss().apply();

    // 定数を更新する
    glUniform1ui(11, static_cast<int>(debug_view_));
    glUniform1f(12, shadow_bias_ * 0.01f);

    // シーンを描画する
    scene.apply(scene::ApplyType::SHADE);
    scene.draw(scene::DrawType::OPAQUE);
  }
}
}  // namespace rtdemo::tech
