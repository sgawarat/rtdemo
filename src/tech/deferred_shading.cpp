#include <rtdemo/tech/deferred_shading.hpp>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(DeferredShading);

bool DeferredShading::restore() {
  // シェーダを生成する
  garie::VertexShader p0_vert =
      util::compile_vertex_shader_from_file(
          "deferred_shading_p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert =
      util::compile_vertex_shader_from_file(
          "deferred_shading_p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag =
  util::compile_fragment_shader_from_file(
          "deferred_shading_p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag =
  util::compile_fragment_shader_from_file(
          "deferred_shading_p1.frag", &log_);
  if (!p1_frag) return false;

  garie::Program p0_prog = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog) return false;

  garie::Program p1_prog = util::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog) return false;

  // リソースを生成する
  const GLuint width = Application::get().screen_width();
  const GLuint height = Application::get().screen_height();

  garie::Texture ds_tex;
  ds_tex.gen();
  ds_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);

  const GLenum format = GL_RGBA8;
  garie::Texture g0_tex;
  g0_tex.gen();
  g0_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

  garie::Texture g1_tex;
  g1_tex.gen();
  g1_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

  garie::Texture g2_tex;
  g2_tex.gen();
  g2_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

  garie::Texture g3_tex;
  g3_tex.gen();
  g3_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

  garie::Framebuffer fbo = garie::FramebufferBuilder()
                                 .depthstencil_texture(ds_tex)
                                 .color_texture(0, g0_tex)
                                 .color_texture(1, g1_tex)
                                 .color_texture(2, g2_tex)
                                 .color_texture(3, g3_tex)
                                 .build();

  garie::Viewport viewport(0.f, 0.f, static_cast<float>(width), static_cast<float>(height));

  garie::Sampler ss = garie::SamplerBuilder()
        .min_filter(GL_NEAREST)
        .mag_filter(GL_NEAREST)
        .build();

  // 後始末
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  ds_tex_ = std::move(ds_tex);
  g0_tex_ = std::move(g0_tex);
  g1_tex_ = std::move(g1_tex);
  g2_tex_ = std::move(g2_tex);
  g3_tex_ = std::move(g3_tex);
  fbo_ = std::move(fbo);
  viewport_ = std::move(viewport);
  ss_ = std::move(ss);
  //debug_view_ = DebugView::DEFAULT;
  log_ = "成功";
  return true;
}

bool DeferredShading::invalidate() {
  p0_prog_ = garie::Program();
  p1_prog_ = garie::Program();
  ds_tex_ = garie::Texture();
  g0_tex_ = garie::Texture();
  g1_tex_ = garie::Texture();
  g2_tex_ = garie::Texture();
  g3_tex_ = garie::Texture();
  fbo_ = garie::Framebuffer();
  ss_ = garie::Sampler();
  log_ = "利用不可";
  return true;
}

void DeferredShading::update() {}

void DeferredShading::update_gui() {
  ImGui::Begin("DeferredShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_), "Default\0Depth\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0Reconstructed position\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void DeferredShading::apply(Scene& scene) {
  // パス0
  {
    // FBOをバインドする
    fbo_.bind(GL_DRAW_FRAMEBUFFER);
    viewport_.apply();

    // レンダターゲットをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f, 0);

    // パイプラインをバインドする
    p0_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

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

    // ライティングパスをバインドする
    p1_prog_.use();
    util::default_rs().apply();
    util::additive_bs().apply();
    util::default_dss().apply();

    // リソースをバインドする
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

    // 定数をアップロードする
    glUniform1i(32, static_cast<int>(debug_view_));

    // ライトボリュームを描画する
    if (debug_view_ == DebugView::DEFAULT) {
      scene.apply(ApplyType::LIGHT);
      scene.draw(DrawType::LIGHT_VOLUME);
    } else {
      util::screen_quad_vao().bind();
      util::draw_screen_quad();
    }
  }
}
}  // namespace rtrdemo::tech
