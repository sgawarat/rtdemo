#include <rtdemo/tech/tiled_forward_shading.hpp>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(TiledForwardShading);

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


  const GLuint width = Application::get().screen_width();
  const GLuint height = Application::get().screen_height();
  const size_t grid_width = (width + 31) / 32;
  const size_t grid_height = (height + 31) / 32;

  // リソースを生成する
  garie::Texture depth_tex;
  depth_tex.gen();
  depth_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);

  garie::Texture rt0_tex;
  rt0_tex.gen();
  rt0_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

  garie::Framebuffer p0_fbo = garie::FramebufferBuilder()
      .depth_texture(depth_tex)
      .build();

  garie::Framebuffer p2_fbo = garie::FramebufferBuilder()
      .depth_texture(depth_tex)
      .color_texture(0, rt0_tex)
      .build();

  garie::Viewport viewport(0.f, 0.f, static_cast<float>(width), static_cast<float>(height));

  garie::Buffer light_grid_ssbo;
  light_grid_ssbo.gen();
  light_grid_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, grid_width * grid_height * sizeof(GridCell), nullptr, 0);
  
  garie::Buffer light_index_ssbo;
  light_index_ssbo.gen();
  light_index_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, (MAX_LIGHT_COUNT * grid_width * grid_height + 1) * sizeof(uint32_t), nullptr, 0);

  // 後始末
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  p2_prog_ = std::move(p2_prog);
  p3_prog_ = std::move(p3_prog);
  depth_tex_ = std::move(depth_tex);
  rt0_tex_ = std::move(rt0_tex);
  p0_fbo_ = std::move(p0_fbo);
  p2_fbo_ = std::move(p2_fbo);
  viewport_ = std::move(viewport);
  light_grid_ssbo_ = std::move(light_grid_ssbo);
  light_index_ssbo_ = std::move(light_index_ssbo);
  grid_width_ = grid_width;
  grid_height_ = grid_height;
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
  ImGui::End();
}

void TiledForwardShading::apply(Scene& scene) {
  // パス0:Pre-Z
  {
    // 深度バッファのみのFBOをバインドする
    p0_fbo_.bind(GL_DRAW_FRAMEBUFFER);
    viewport_.apply();

    // 深度バッファをクリアする
    util::clear(1.f);

    // パイプラインをバインドする
    p0_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // シーンを描画する
    scene.apply(ApplyType::NO_SHADE);
    scene.draw(DrawType::OPAQUE);
  }

  // パス1:ライト割り当て
  {
    // パイプラインをバインドする
    p1_prog_.use();

    // リソースをバインドする
    depth_tex_.active(0, GL_TEXTURE_2D);
    light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 8);
    light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 9);

    // ディスパッチ
    scene.apply(ApplyType::LIGHT);
    glDispatchCompute(static_cast<GLuint>(grid_width_), static_cast<GLuint>(grid_height_), 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // パス2:シェーディング
  {
    // 深度とカラーを持つFBOをバインドする
    p2_fbo_.bind(GL_DRAW_FRAMEBUFFER);

    // レンダターゲットをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f});

    // パイプラインをバインドする
    p2_prog_.use();
    util::default_rs().apply();
    util::alpha_blending_bs().apply();
    util::depth_test_no_write_dss().apply();

    // リソースをバインドする
    light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 8);
    light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 9);

    // 定数をアップロードする
    glUniform1ui(32, static_cast<int>(debug_view_));  

    // シーンを描画する
    scene.apply(ApplyType::SHADE);
    scene.draw(DrawType::OPAQUE);
  }

  // パス3:ポストプロセッシング
  {
    // バックバッファをフレームバッファにバインドする
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    util::screen_viewport().apply();

    // バックバッファをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);

    // パイプラインをバインドする
    p3_prog_.use();
    util::default_rs().apply();
    util::default_bs().apply();
    util::default_dss().apply();

    // リソースをバインドする
    rt0_tex_.active(8, GL_TEXTURE_2D);

    // 描画する
    util::screen_quad_vao().bind();
    util::draw_screen_quad();
  }
}
}  // namespace rtrdemo::tech
