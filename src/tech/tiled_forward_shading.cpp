#include <rtdemo/tech/tiled_forward_shading.hpp>
#include <imgui.h>
#include <gsl/gsl>
#include <glm/glm.hpp>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(TiledForwardShading);

bool TiledForwardShading::restore() {
  // 成功しなければ、リソースを破棄するように設定する
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) invalidate();
  });

  // シェーダを生成する
  garie::VertexShader p0_vert = util::compile_vertex_shader_from_file(
      "tiled_forward_shading/p0.vert", &log_);
  if (!p0_vert) return false;

  garie::FragmentShader p0_frag = util::compile_fragment_shader_from_file(
    "tiled_forward_shading/p0.frag", &log_);
  if (!p0_frag) return false;

  garie::ComputeShader p1_comp = util::compile_compute_shader_from_file(
    "tiled_forward_shading/p1.comp", &log_);
  if (!p1_comp) return false;

  garie::VertexShader p2_vert = util::compile_vertex_shader_from_file(
    "tiled_forward_shading/p2.vert", &log_);
  if (!p2_vert) return false;

  garie::FragmentShader p2_frag = util::compile_fragment_shader_from_file(
    "tiled_forward_shading/p2.frag", &log_);
  if (!p2_frag) return false;

  garie::VertexShader p3_vert = util::compile_vertex_shader_from_file(
    "tiled_forward_shading/p3.vert", &log_);
  if (!p3_vert) return false;

  garie::FragmentShader p3_frag = util::compile_fragment_shader_from_file(
    "tiled_forward_shading/p3.frag", &log_);
  if (!p3_frag) return false;

  // プログラムを生成する
  p0_prog_ = util::link_program(p0_vert, p0_frag, &log_);
  if (!p0_prog_) return false;

  p1_prog_ = util::link_program(p1_comp, &log_);
  if (!p1_prog_) return false;

  p2_prog_ = util::link_program(p2_vert, p2_frag, &log_);
  if (!p2_prog_) return false;

  p3_prog_ = util::link_program(p3_vert, p3_frag, &log_);
  if (!p3_prog_) return false;

  // スクリーンを占めるタイル数を計算する
  const uint32_t screen_width = Application::get().screen_width();
  const uint32_t screen_height = Application::get().screen_height();
  tiled_screen_width_ = (screen_width + TILE_WIDTH - 1) / TILE_WIDTH;
  tiled_screen_height_ = (screen_height + TILE_HEIGHT - 1) / TILE_HEIGHT;
  const uint32_t tiled_screen_size = tiled_screen_width_ * tiled_screen_height_;

  // リソースを生成する
  depth_tex_.gen();
  depth_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, screen_width, screen_height);

  rt0_tex_.gen();
  rt0_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screen_width, screen_height);

  p0_fbo_ = garie::FramebufferBuilder()
      .depth_texture(depth_tex_)
      .build();

  p2_fbo_ = garie::FramebufferBuilder()
      .depth_texture(depth_tex_)
      .color_texture(0, rt0_tex_)
      .build();

  viewport_ = garie::Viewport(0.f, 0.f, static_cast<float>(screen_width), static_cast<float>(screen_height));

  constant_ubo_.gen();
  constant_ubo_.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, sizeof(Constant), nullptr, GL_MAP_WRITE_BIT);

  tiles_ssbo_.gen();
  tiles_ssbo_.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, tiled_screen_size * sizeof(Tile), nullptr, 0);

  light_indices_ssbo_.gen();
  light_indices_ssbo_.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, MAX_LIGHT_COUNT * tiled_screen_size * sizeof(uint32_t), nullptr, 0);

  light_index_count_ssbo_.gen();
  light_index_count_ssbo_.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), nullptr, 0);

  print_ssbo_.gen();
  print_ssbo_.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, tiled_screen_size * sizeof(Print), nullptr, 0);

  log_ = "成功";

  succeeded = true;
  return true;
}

bool TiledForwardShading::invalidate() {
  p0_prog_.del();
  p1_prog_.del();
  p2_prog_.del();
  p3_prog_.del();
  depth_tex_.del();
  rt0_tex_.del();
  p0_fbo_.del();
  p2_fbo_.del();
  constant_ubo_.del();
  tiles_ssbo_.del();
  light_indices_ssbo_.del();
  light_index_count_ssbo_.del();
  print_ssbo_.del();
  log_ = "利用不可";
  return true;
}

void TiledForwardShading::update() {
  auto& app = Application::get();

  constant_ubo_.bind(GL_UNIFORM_BUFFER);
  auto constant = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Constant), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (constant) {
    constant->tile_count[0] = (app.screen_width() + TILE_WIDTH - 1) / TILE_WIDTH;
    constant->tile_count[1] = (app.screen_height() + TILE_HEIGHT - 1) / TILE_HEIGHT;
    constant->pixel_count[0] = app.screen_width();
    constant->pixel_count[1] = app.screen_height();
    constant->mode = mode_;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }
}

void TiledForwardShading::update_gui() {
  ImGui::Begin("TiledForwardShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&mode_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0TileIndex\0TileLightCount\0Shaded\0");
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

    // リソースをバインドする
    constant_ubo_.bind_base(GL_UNIFORM_BUFFER, 15);

    // シーンを描画する
    scene.apply(ApplyType::NO_SHADE);
    scene.draw(DrawType::OPAQUE);
  }

  // パス1:ライト割り当て
  {
    // パイプラインをバインドする
    p1_prog_.use();

    // リソースをバインドする
    constant_ubo_.bind_base(GL_UNIFORM_BUFFER, 15);
    depth_tex_.active(8, GL_TEXTURE_2D);
    tiles_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 8);
    light_indices_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 9);
    light_index_count_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 10);
    print_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 15);

    // ディスパッチ
    scene.apply(ApplyType::LIGHT);
    glDispatchCompute(tiled_screen_width_, tiled_screen_height_, 1);
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
    constant_ubo_.bind_base(GL_UNIFORM_BUFFER, 15);
    tiles_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 8);
    light_indices_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 9);
    light_index_count_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 10);

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
    constant_ubo_.bind_base(GL_UNIFORM_BUFFER, 15);
    rt0_tex_.active(8, GL_TEXTURE_2D);

    // 描画する
    util::screen_quad_vao().bind();
    util::draw_screen_quad();
  }
}
}  // namespace rtrdemo::tech
