#include <rtdemo/tech/tiled_forward_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
namespace {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, TiledForwardShading);
}  // namespace

bool TiledForwardShading::restore() {
  // シェーダを生成する
  garie::VertexShader p0_vert = util::compile_vertex_shader_from_file(
      "assets/shaders/tiled_forward_shading_p0.vert", &log_);
  if (!p0_vert) return false;

  garie::FragmentShader p0_frag = util::compile_fragment_shader_from_file(
    "assets/shaders/tiled_forward_shading_p0.frag", &log_);
  if (!p0_frag) return false;

  garie::Program p0_prog = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog) return false;

  garie::ComputeShader p1_comp = util::compile_compute_shader_from_file(
    "assets/shaders/tiled_forward_shading_p1.comp", &log_);
  if (!p1_comp) return false;

  garie::Program p1_prog = util::link_program(p1_comp, &log_);
  if (!p1_prog) return false;

  garie::VertexShader p2_vert = util::compile_vertex_shader_from_file(
    "assets/shaders/tiled_forward_shading_p2.vert", &log_);
  if (!p2_vert) return false;

  garie::FragmentShader p2_frag = util::compile_fragment_shader_from_file(
    "assets/shaders/tiled_forward_shading_p2.frag", &log_);
  if (!p2_frag) return false;

  garie::Program p2_prog = util::link_program(p2_vert, p2_frag, &log_);
  if (!p2_prog) return false;

  garie::VertexShader p3_vert = util::compile_vertex_shader_from_file(
    "assets/shaders/tiled_forward_shading_p3.vert", &log_);
  if (!p3_vert) return false;

  garie::FragmentShader p3_frag = util::compile_fragment_shader_from_file(
    "assets/shaders/tiled_forward_shading_p3.frag", &log_);
  if (!p3_frag) return false;

  garie::Program p3_prog = util::link_program(p3_vert, p3_frag, &log_);
  if (!p3_prog) return false;


  const size_t grid_width = (1280 + 31) / 32;
  const size_t grid_height = (720 + 31) / 32;

  // リソースを生成する
  garie::Texture depth_tex;
  depth_tex.gen();
  depth_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 1280, 720);

  garie::Texture rt0_tex;
  rt0_tex.gen();
  rt0_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1280, 720);

  garie::Framebuffer p0_fbo = garie::FramebufferBuilder()
      .depth_texture(depth_tex)
      .build();

  garie::Framebuffer p2_fbo = garie::FramebufferBuilder()
      .depth_texture(depth_tex)
      .color_texture(0, rt0_tex)
      .build();

  garie::Buffer light_grid_ssbo;
  light_grid_ssbo.gen();
  light_grid_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, grid_width * grid_height * sizeof(GridCell), nullptr, 0);
  
  garie::Buffer light_index_ssbo;
  light_index_ssbo.gen();
  light_index_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, (200 * grid_width * grid_height + 1) * sizeof(uint32_t), nullptr, 0);

  // 後始末
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  p2_prog_ = std::move(p2_prog);
  p3_prog_ = std::move(p3_prog);
  depth_tex_ = std::move(depth_tex);
  rt0_tex_ = std::move(rt0_tex);
  p0_fbo_ = std::move(p0_fbo);
  p2_fbo_ = std::move(p2_fbo);
  light_grid_ssbo_ = std::move(light_grid_ssbo);
  light_index_ssbo_ = std::move(light_index_ssbo);
  log_ = "成功";
  return true;
}

bool TiledForwardShading::invalidate() {
  p0_prog_ = garie::Program();
  p1_prog_ = garie::Program();
  p2_prog_ = garie::Program();
  p3_prog_ = garie::Program();
  depth_tex_ = garie::Texture();
  rt0_tex_ = garie::Texture();
  p0_fbo_ = garie::Framebuffer();
  p2_fbo_ = garie::Framebuffer();
  light_grid_ssbo_ = garie::Buffer();
  light_index_ssbo_ = garie::Buffer();
  log_ = "利用不可";
  return true;
}

void TiledForwardShading::update() {}

void TiledForwardShading::update_gui() {
  ImGui::Begin("TiledForwardShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0TileIndex\0TileLightCount\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::TextWrapped("grid = (%d, %d)", cell_.first, cell_.count);
  ImGui::End();
}

void TiledForwardShading::apply(scene::Scene& scene) {
  // パス0:Pre-Z
  {
    // 深度バッファのみのFBOをバインドする
    p0_fbo_.bind(GL_DRAW_FRAMEBUFFER);
    const GLenum p0_draw_buffers[] = {
      GL_NONE,
      GL_NONE,
      GL_NONE,
      GL_NONE,
    };
    glDrawBuffers(4, p0_draw_buffers);

    // 深度バッファをクリアする
    glDepthMask(GL_TRUE);
    glClearDepthf(1.f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // バインド
    p0_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // シーンを描画する
    scene.apply(scene::ApplyType::NO_SHADE);
    #undef OPAQUE
    scene.draw(scene::DrawType::OPAQUE);
  }

  // パス1:ライト割り当て
  {
    // バインド
    p1_prog_.use();
    depth_tex_.active(0, GL_TEXTURE_2D);
    light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 20);
    light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 21);

    // ディスパッチ
    scene.apply(scene::ApplyType::LIGHT);
    glDispatchCompute(40, 23, 1);
    //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // パス2:シェーディング
  {
    // 深度とカラーを持つFBOをバインドする
    p2_fbo_.bind(GL_DRAW_FRAMEBUFFER);
    const GLenum p2_draw_buffers[] = {
      GL_COLOR_ATTACHMENT0,
      GL_NONE,
      GL_NONE,
      GL_NONE,
    };
    glDrawBuffers(4, p2_draw_buffers);

    // レンダターゲットをクリアする
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // パイプラインをバインドする
    p2_prog_.use();
    glUniform1ui(11, static_cast<int>(debug_view_));  
    light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 10);
    light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 11);
    util::default_rs().apply();
    util::alpha_blending_bs().apply();
    util::depth_test_no_write_dss().apply();

    // シーンを描画する
    scene.apply(scene::ApplyType::SHADE);
    #undef OPAQUE
    scene.draw(scene::DrawType::OPAQUE);
  }

  // パス3:ポストプロセッシング
  {
    // バックバッファをフレームバッファにバインドする
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    const GLenum p3_draw_buffers[] = {
      GL_BACK_LEFT,
      GL_NONE,
      GL_NONE,
      GL_NONE,
    };
    glDrawBuffers(4, p3_draw_buffers);

    // バインド
    p3_prog_.use();
    rt0_tex_.active(0, GL_TEXTURE_2D);
    util::default_rs().apply();
    util::default_bs().apply();
    util::default_dss().apply();

    // 描画する
    util::screen_quad_vao().bind();
    util::draw_screen_quad();
  }
}
}  // namespace rtrdemo::tech
