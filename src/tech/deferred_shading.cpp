#include <rtdemo/tech/deferred_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/tech/detail/util.hpp>

namespace rtdemo {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, DeferredShading);

namespace tech {
bool DeferredShading::restore() {
  garie::VertexShader p0_vert =
      detail::compile_shader_from_file<GL_VERTEX_SHADER>(
          "assets/shaders/deferred_shading_p0.vert", &log_);
  if (!p0_vert) return false;

  garie::VertexShader p1_vert =
      detail::compile_shader_from_file<GL_VERTEX_SHADER>(
          "assets/shaders/deferred_shading_p1.vert", &log_);
  if (!p1_vert) return false;

  garie::FragmentShader p0_frag =
      detail::compile_shader_from_file<GL_FRAGMENT_SHADER>(
          "assets/shaders/deferred_shading_p0.frag", &log_);
  if (!p0_frag) return false;

  garie::FragmentShader p1_frag =
      detail::compile_shader_from_file<GL_FRAGMENT_SHADER>(
          "assets/shaders/deferred_shading_p1.frag", &log_);
  if (!p1_frag) return false;

  garie::Program p0_prog = detail::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog) return false;

  garie::Program p1_prog = detail::link_program(p1_vert, p1_frag, &log_);
  if (!p1_prog) return false;

  garie::Texture ds_tex;
  ds_tex.gen();
  ds_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 1280, 720);

  garie::Texture g0_tex;
  g0_tex.gen();
  g0_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);

  garie::Texture g1_tex;
  g1_tex.gen();
  g1_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);

  garie::Texture g2_tex;
  g2_tex.gen();
  g2_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);

  garie::Texture g3_tex;
  g3_tex.gen();
  g3_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);

  garie::Framebuffer fbo = garie::FramebufferBuilder()
                                 .depthstencil_texture(ds_tex)
                                 .color_texture(0, g0_tex)
                                 .color_texture(1, g1_tex)
                                 .color_texture(2, g2_tex)
                                 .color_texture(3, g3_tex)
                                 .build();

  garie::Sampler ss = garie::SamplerBuilder()
        .min_filter(GL_LINEAR)
        .mag_filter(GL_LINEAR)
        .build();

  if (!detail::screen_quad_vao()) {
    log_ = "failed to init screen-quad geometry";
    RT_LOG(error, "Failed to init screen-quad");
    return false;
  }

  // finalize
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  ds_tex_ = std::move(ds_tex);
  g0_tex_ = std::move(g0_tex);
  g1_tex_ = std::move(g1_tex);
  g2_tex_ = std::move(g2_tex);
  g3_tex_ = std::move(g3_tex);
  fbo_ = std::move(fbo);
  ss_ = std::move(ss);
  log_ = "succeeded";
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
  log_ = "not available";
  return true;
}

void DeferredShading::update() {}

void DeferredShading::update_gui() {
  ImGui::TextWrapped("%s", log_.c_str());
}

void DeferredShading::apply(scene::Scene* scene) {
  if (!p0_prog_ || !p1_prog_) return;

  fbo_.bind(GL_DRAW_FRAMEBUFFER);
  const GLenum draw_buffers[] = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3,
  };
  glDrawBuffers(4, draw_buffers);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(GL_TRUE);
  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClearDepthf(1.f);
  glClearStencil(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Geometry pass
  p0_prog_.use();
  detail::default_rs().apply();
  detail::alpha_blending_bs().apply();
  detail::depth_test_dss().apply();
  if (scene) {
    scene->apply(Layout::ResourcePass::ALL);
    scene->draw(Layout::DrawPass::GEOMETRIES);
  }

  // Lighting pass
  p1_prog_.use();
  ds_tex_.active(0, GL_TEXTURE_2D);
  ss_.bind(0);
  g0_tex_.active(1, GL_TEXTURE_2D);
  ss_.bind(1);
  g1_tex_.active(2, GL_TEXTURE_2D);
  ss_.bind(2);
  g2_tex_.active(3, GL_TEXTURE_2D);
  ss_.bind(3);
  g3_tex_.active(4, GL_TEXTURE_2D);
  ss_.bind(4);
  detail::default_rs().apply();
  detail::additive_bs().apply();
  detail::default_dss().apply();
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  const GLenum restored_draw_buffers[] = {
      GL_BACK_LEFT, GL_NONE, GL_NONE, GL_NONE,
  };
  glDrawBuffers(4, restored_draw_buffers);
  detail::screen_quad_vao().bind();
  detail::draw_screen_quad();
}
}  // namespace tech
}  // namespace rtrdemo