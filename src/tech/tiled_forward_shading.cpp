#include <rtdemo/tech/tiled_forward_shading.hpp>
#include <fstream>
#include <vector>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/managed.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo {
RT_MANAGED_TECHNIQUE_INSTANCE(tech, TiledForwardShading);

namespace tech {
bool TiledForwardShading::restore() {
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

  const size_t grid_width = (1280 + 31) / 32;
  const size_t grid_height = (720 + 31) / 32;

  garie::Buffer light_grid_ssbo;
  light_grid_ssbo.gen();
  light_grid_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, grid_width * grid_height * sizeof(GridCell), nullptr, GL_MAP_WRITE_BIT);
  
  garie::Buffer light_index_ssbo;
  light_index_ssbo.gen();
  light_index_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, 200 * grid_width * grid_height * sizeof(uint32_t), nullptr, GL_MAP_WRITE_BIT);

  // finalize
  p0_prog_ = std::move(p0_prog);
  p1_prog_ = std::move(p1_prog);
  p2_prog_ = std::move(p2_prog);
  light_grid_ssbo_ = std::move(light_grid_ssbo);
  light_index_ssbo_ = std::move(light_index_ssbo);
  log_ = "succeeded";
  return true;
}

bool TiledForwardShading::invalidate() {
  p0_prog_ = garie::Program();
  p1_prog_ = garie::Program();
  p2_prog_ = garie::Program();
  light_grid_ssbo_ = garie::Buffer();
  light_index_ssbo_ = garie::Buffer();
  log_ = "not available";
  return true;
}

void TiledForwardShading::update() {}

void TiledForwardShading::update_gui() {
  ImGui::Begin("TiledForwardShading");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&debug_view_), "Default\0Position\0Normal\0Ambient\0Diffuse\0Specular\0SpecularPower\0TileIndex\0");
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void TiledForwardShading::apply(scene::Scene& scene) {
  // pre-z
  p0_prog_.use();
  util::default_rs().apply();
  util::default_bs().apply();
  util::depth_test_dss().apply();
  scene.apply(scene::ApplyType::NO_SHADE);
  scene.draw(scene::DrawType::OPAQUE);

  // light assignment
  p1_prog_.use();
  light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 20);
  light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 21);
  scene.apply(scene::ApplyType::LIGHT);
  glDispatchCompute(40, 23, 0);

  // shading
  p2_prog_.use();
  glUniform1ui(11, static_cast<int>(debug_view_));  
  light_grid_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 20);
  light_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 21);
  util::default_rs().apply();
  util::alpha_blending_bs().apply();
  util::depth_test_no_write_dss().apply();
  scene.apply(scene::ApplyType::SHADE);
  scene.draw(scene::DrawType::OPAQUE);
}
}  // namespace tech
}  // namespace rtrdemo
