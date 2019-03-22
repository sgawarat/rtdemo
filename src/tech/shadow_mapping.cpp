#include <rtdemo/tech/shadow_mapping.hpp>
#include <gsl/gsl>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(ShadowMapping);

bool ShadowMapping::restore() {
  // 成功しなければ、リソースを破棄するように設定する
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) invalidate();
  });

  // シェーダを読み込む
  garie::VertexShader p0_vert = util::compile_vertex_shader_from_file(
      "shadow_mapping/p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert = util::compile_vertex_shader_from_file(
      "shadow_mapping/p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag = util::compile_fragment_shader_from_file(
      "shadow_mapping/p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag = util::compile_fragment_shader_from_file(
      "shadow_mapping/p1.frag", &log_);
  if (!p1_frag) return false;

  // プログラムを生成する
  p0_prog_ = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog_) return false;

  p1_prog_ = util::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog_) return false;

  // リソースを生成する
  constant_ub_.gen();
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, sizeof(Constant), nullptr, GL_MAP_WRITE_BIT);

  const GLuint shadow_map_width = 1024;
  const GLuint shadow_map_height = 1024;
  depth_tex_.gen();
  depth_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, shadow_map_width, shadow_map_height);

  p0_fbo_ =
      garie::FramebufferBuilder().depth_texture(depth_tex_).build();

  p0_viewport_ = garie::Viewport(0.f, 0.f, shadow_map_width, shadow_map_height);

  const float border_color[4] = {1.f, 1.f, 1.f, 1.f};
  ss_ = garie::SamplerBuilder()
      .min_filter(GL_NEAREST)
      .mag_filter(GL_NEAREST)
      .wrap_s(GL_CLAMP_TO_BORDER)
      .wrap_t(GL_CLAMP_TO_BORDER)
      .border_color(border_color)
      .build();

  log_ = "成功";

  succeeded = true;
  return true;
}

bool ShadowMapping::invalidate() {
  ss_.del();
  p0_fbo_.del();
  depth_tex_.del();
  constant_ub_.del();
  p0_prog_.del();
  p1_prog_.del();
  log_ = "利用不可";
  return true;
}

void ShadowMapping::update() {
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  auto constant = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Constant), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (constant) {
    *constant = constant_;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }
}

void ShadowMapping::update_gui() {
  ImGui::Begin("ShadowMapping");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&constant_.mode),
               "Default\0SHADOWED\0");
  // ImGui::DragFloat("Bias * 100", &shadow_bias_, 0.01f, -1.f, 1.f);
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void ShadowMapping::apply(Scene& scene) {
  // パス0:シャドウ
  {
    // 深度バッファのみのFBOをバインドする
    p0_fbo_.bind(GL_DRAW_FRAMEBUFFER);
    p0_viewport_.apply();

    // 深度バッファをクリアする
    util::clear(1.f);

    //  パイプラインをバインドする
    p0_prog_.use();
    util::default_rs().apply();
    // util::backface_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);

    // シーンを描画する
    scene.apply(ApplyType::SHADOW);
    scene.draw(DrawType::OPAQUE);
  }

  // パス1:シェーディング
  {
    // バックバッファをターゲットにする
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    util::screen_viewport().apply();

    // バックバッファをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);

    // パイプラインをバインドする
    p1_prog_.use();
    util::default_rs().apply();
    util::alpha_blending_bs().apply();
    util::depth_test_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);
    depth_tex_.active(8, GL_TEXTURE_2D);
    ss_.bind(8);

    // シーンを描画する
    scene.apply(ApplyType::SHADE);
    scene.draw(DrawType::OPAQUE);
  }
}
}  // namespace rtdemo::tech
