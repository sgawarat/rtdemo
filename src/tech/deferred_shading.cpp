#include <rtdemo/tech/deferred_shading.hpp>
#include <gsl/gsl>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(DeferredShading);

bool DeferredShading::restore() {
  // 成功しなければ、リソースを破棄するように設定する
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) invalidate();
  });

  // シェーダを生成する
  garie::VertexShader p0_vert =
      util::compile_vertex_shader_from_file(
          "deferred_shading/p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert =
      util::compile_vertex_shader_from_file(
          "deferred_shading/p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag =
  util::compile_fragment_shader_from_file(
          "deferred_shading/p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag =
  util::compile_fragment_shader_from_file(
          "deferred_shading/p1.frag", &log_);
  if (!p1_frag) return false;

  // プログラムを生成する
  p0_prog_ = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog_) return false;

  p1_prog_ = util::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog_) return false;

  // リソースを生成する
  constant_ub_.gen();
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, 1024, nullptr, GL_MAP_WRITE_BIT);

  const uint32_t screen_width = Application::get().screen_width();
  const uint32_t screen_height = Application::get().screen_height();
  ds_tex_.gen();
  ds_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, screen_width, screen_height);

  g0_tex_.gen();
  g0_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screen_width, screen_height);

  g1_tex_.gen();
  g1_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screen_width, screen_height);

  g2_tex_.gen();
  g2_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screen_width, screen_height);

  g3_tex_.gen();
  g3_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screen_width, screen_height);

  fb_ = garie::FramebufferBuilder()
      .depthstencil_texture(ds_tex_)
      .color_texture(0, g0_tex_)
      .color_texture(1, g1_tex_)
      .color_texture(2, g2_tex_)
      .color_texture(3, g3_tex_)
      .build();

  viewport_ = garie::Viewport(0.f, 0.f, static_cast<float>(screen_width), static_cast<float>(screen_height));

  ss_ = garie::SamplerBuilder()
        .min_filter(GL_NEAREST)
        .mag_filter(GL_NEAREST)
        .build();

  log_ = "成功";

  succeeded = true;
  return true;
}

bool DeferredShading::invalidate() {
  p0_prog_.del();
  p1_prog_.del();
  constant_ub_.del();
  ds_tex_.del();
  g0_tex_.del();
  g1_tex_.del();
  g2_tex_.del();
  g3_tex_.del();
  fb_.del();
  ss_.del();
  log_ = "利用不可";
  return true;
}

void DeferredShading::update() {
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  auto constant = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Mode), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (constant) {
    constant->mode = mode_;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }
}

void DeferredShading::update_gui() {
  ImGui::Begin("DeferredShading");
  ImGui::Combo("mode", reinterpret_cast<int*>(&mode_), "Default\0Depth\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0Reconstructed position\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void DeferredShading::apply(Scene& scene) {
  // パス0
  {
    // Gバッファをレンダターゲットにバインドする
    fb_.bind(GL_DRAW_FRAMEBUFFER);
    viewport_.apply();

    // レンダターゲットをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f, 0);

    // パイプラインをバインドする
    p0_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);

    // シーンを描画する
    scene.apply(ApplyType::SHADE);
    scene.draw(DrawType::OPAQUE);
  }

  // パス1
  {
    // MRTを解除する
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    util::screen_viewport().apply();

    // バックバッファをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);

    // パイプラインをバインドする
    p1_prog_.use();
    util::default_rs().apply();
    util::additive_bs().apply();
    util::default_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);
    ds_tex_.active(8, GL_TEXTURE_2D);
    ss_.bind(8);
    g0_tex_.active(9, GL_TEXTURE_2D);
    ss_.bind(9);
    g1_tex_.active(10, GL_TEXTURE_2D);
    ss_.bind(10);
    g2_tex_.active(11, GL_TEXTURE_2D);
    ss_.bind(11);
    g3_tex_.active(12, GL_TEXTURE_2D);
    ss_.bind(12);

    // ライトボリュームを描画する
    if (mode_ == Mode::DEFAULT) {
      scene.apply(ApplyType::LIGHT);
      scene.draw(DrawType::LIGHT_VOLUME);
    } else {
      // テクスチャの内容を表示するためにfullscreen quadを描画する
      // TODO:専用のシェーダを用意する
      util::screen_quad_vao().bind();
      util::draw_screen_quad();
    }
  }
}
}  // namespace rtrdemo::tech
