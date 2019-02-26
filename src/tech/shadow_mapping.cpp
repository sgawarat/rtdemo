#include <rtdemo/tech/shadow_mapping.hpp>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(ShadowMapping);

bool ShadowMapping::restore() {
  // シェーダを読み込む
  garie::VertexShader p0_vert = util::compile_vertex_shader_from_file(
      "shadow_mapping_p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert = util::compile_vertex_shader_from_file(
      "shadow_mapping_p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag = util::compile_fragment_shader_from_file(
      "shadow_mapping_p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag = util::compile_fragment_shader_from_file(
      "shadow_mapping_p1.frag", &log_);
  if (!p1_frag) return false;

  garie::Program p0_prog = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog) return false;

  garie::Program p1_prog = util::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog) return false;

  // リソースを生成する
  garie::Buffer ub;
  ub.gen();
  ub.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, 1024, nullptr, GL_MAP_WRITE_BIT);

  const GLuint width = 1024;
  const GLuint height = 1024;
  garie::Texture depth_tex;
  depth_tex.gen();
  depth_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, width, height);

  garie::Framebuffer p0_fbo =
      garie::FramebufferBuilder().depth_texture(depth_tex).build();

  garie::Viewport p0_viewport(0.f, 0.f, width, height);

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
  ub_ = std::move(ub);
  depth_tex_ = std::move(depth_tex);
  p0_fbo_ = std::move(p0_fbo);
  p0_viewport_ = std::move(p0_viewport);
  ss_ = std::move(ss);
  log_ = "成功";
  return true;
}

bool ShadowMapping::invalidate() {
  ss_ = {};
  p0_fbo_ = {};
  depth_tex_ = {};
  ub_ = {};
  p0_prog_ = {};
  p1_prog_ = {};
  log_ = "利用不可";
  return true;
}

void ShadowMapping::update() {
  ub_.bind(GL_UNIFORM_BUFFER);
  auto p = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Constant), GL_MAP_WRITE_BIT));
  if (p) {
    p->mode = mode_;
    p->shadow_bias = shadow_bias_ * 0.01f;
  }
  glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void ShadowMapping::update_gui() {
  ImGui::Begin("ShadowMapping");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&mode_),
               "Default\0SHADOWED\0CASTER\0");
  ImGui::DragFloat("Bias * 100", &shadow_bias_, 0.01f, -1.f, 1.f);
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
    util::default_bs().apply();
    util::depth_test_dss().apply();

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
    ub_.bind_base(GL_UNIFORM_BUFFER, 8);
    depth_tex_.active(8, GL_TEXTURE_2D);
    ss_.bind(8);

    // シーンを描画する
    scene.apply(ApplyType::SHADE);
    scene.draw(DrawType::OPAQUE);
  }
}
}  // namespace rtdemo::tech
