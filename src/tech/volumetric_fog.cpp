#include <rtdemo/tech/volumetric_fog.hpp>
#include <imgui.h>
#include <gsl/gsl>
#include <glm/glm.hpp>
#include <rtdemo/logging.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::tech {
RT_MANAGED_TECHNIQUE(VolumetricFog);

bool VolumetricFog::restore() {
  // 成功しなければ、リソースを破棄するように設定する
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) invalidate();
  });

  constant_ = Constant{};

  // シェーダを生成する
  garie::VertexShader shadow_vert = util::compile_vertex_shader_from_file(
    "volumetric_fog/shadow.vert", &log_);
  if (!shadow_vert) return false;

  garie::FragmentShader shadow_frag = util::compile_fragment_shader_from_file(
    "volumetric_fog/shadow.frag", &log_);
  if (!shadow_frag) return false;

  garie::ComputeShader p0_comp = util::compile_compute_shader_from_file(
      "volumetric_fog/p0.comp", &log_);
  if (!p0_comp) return false;

  garie::ComputeShader p1_comp = util::compile_compute_shader_from_file(
    "volumetric_fog/p1.comp", &log_);
  if (!p1_comp) return false;

  garie::VertexShader p2_vert = util::compile_vertex_shader_from_file(
    "volumetric_fog/p2.vert", &log_);
  if (!p2_vert) return false;

  garie::FragmentShader p2_frag = util::compile_fragment_shader_from_file(
    "volumetric_fog/p2.frag", &log_);
  if (!p2_frag) return false;

  // プログラムを生成する
  shadow_prog_ = util::link_program(shadow_vert, shadow_frag, &log_);
  if (!shadow_prog_) return false;

  p0_prog_ = util::link_program(p0_comp, &log_);
  if (!p0_prog_) return false;

  p1_prog_ = util::link_program(p1_comp, &log_);
  if (!p1_prog_) return false;

  p2_prog_ = util::link_program(p2_vert, p2_frag, &log_);
  if (!p2_prog_) return false;

  // froxelの数を計算する
  const uint32_t screen_width = Application::get().screen_width();
  const uint32_t screen_height = Application::get().screen_height();
  constant_.froxel_count[0] = 64;
  constant_.froxel_count[1] = 64;
  constant_.froxel_count[2] = 64;

  const uint32_t shadow_width = 1024;
  const uint32_t shadow_height = 1024;

  // リソースを生成する
  constant_ub_.gen();
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(GL_UNIFORM_BUFFER, sizeof(Constant), nullptr, GL_MAP_WRITE_BIT);

  shadow_tex_.gen();
  shadow_tex_.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, shadow_width, shadow_height);

  vbuffer_tex_.gen();
  vbuffer_tex_.bind(GL_TEXTURE_3D);
  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, constant_.froxel_count[0], constant_.froxel_count[1], constant_.froxel_count[2]);  // TODO:RGBA16Fを使う

  lighting_tex_.gen();
  lighting_tex_.bind(GL_TEXTURE_3D);
  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, constant_.froxel_count[0], constant_.froxel_count[1], constant_.froxel_count[2]);

  lighting_ss_ = garie::SamplerBuilder()
      .min_filter(GL_LINEAR_MIPMAP_NEAREST)
      .mag_filter(GL_LINEAR_MIPMAP_NEAREST)
      // .min_filter(GL_NEAREST)
      // .mag_filter(GL_NEAREST)
      .wrap_s(GL_CLAMP_TO_EDGE)
      .wrap_t(GL_CLAMP_TO_EDGE)
      .wrap_r(GL_CLAMP_TO_EDGE)
      .build();

  shadow_ss_ = garie::SamplerBuilder()
      .min_filter(GL_LINEAR_MIPMAP_NEAREST)
      .mag_filter(GL_LINEAR_MIPMAP_NEAREST)
      .wrap_s(GL_CLAMP_TO_EDGE)
      .wrap_t(GL_CLAMP_TO_EDGE)
      .wrap_r(GL_CLAMP_TO_EDGE)
      .build();

  shadow_fb_ =
      garie::FramebufferBuilder().depth_texture(shadow_tex_).build();

  shadow_vp_ = garie::Viewport(0.f, 0.f, shadow_width, shadow_height);

  log_ = "成功";

  succeeded = true;
  return true;
}

bool VolumetricFog::invalidate() {
  shadow_prog_.del();
  p0_prog_.del();
  p1_prog_.del();
  p2_prog_.del();
  shadow_tex_.del();
  vbuffer_tex_.del();
  lighting_tex_.del();
  lighting_ss_.del();
  log_ = "利用不可";
  return true;
}

void VolumetricFog::update() {
  auto& app = Application::get();

  // 定数用バッファを更新する
  constant_ub_.bind(GL_UNIFORM_BUFFER);
  auto constant = reinterpret_cast<Constant*>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(Constant), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (constant) {
    *constant = constant_;
    glUnmapBuffer(GL_UNIFORM_BUFFER);
  }
}

void VolumetricFog::update_gui() {
  ImGui::Begin("VolumetricFog");
  ImGui::Combo("debug view", reinterpret_cast<int*>(&constant_.mode), "Default\0POSITION\0NORMAL\0AMBIENT\0DIFFUSE\0SPECULAR\0SPECULAR_POWER\0VTEXCOORD\0SCATTERING\0TRANSMITTANCE\0VLIGHTING\0");
  ImGui::SliderFloat("depth scale", &constant_.volume_depth_scale, 0.f, 30.f);
  ImGui::SliderFloat("depth offset", &constant_.volume_depth_offset, -10.f, 10.f);
  ImGui::SliderFloat3("sigma_s", constant_.scattering_coeff, 0.f, 10.f);
  ImGui::SliderFloat("sigma_a", &absorption_coeff_, 0.f, 10.f);
  constant_.extinction_coeff = absorption_coeff_ + *std::max_element(constant_.scattering_coeff, constant_.scattering_coeff + 3);
  ImGui::SliderFloat("1/atten", &constant_.attenuation_coeff, 0.f, 100.f);
  ImGui::Combo("fog shape", reinterpret_cast<int*>(&constant_.fog_shape), "Height\0Box\0Sphere\0");
  ImGui::SliderFloat("fog radius", &constant_.fog_radius, 0.f, 20.f);
  ImGui::SliderFloat3("fog center", constant_.fog_center, -10.f, 10.f);
  ImGui::SliderFloat("fog boundary", &constant_.fog_boundary, 1.f, 20.f);
  ImGui::TextWrapped("%s", log_.c_str());
  ImGui::End();
}

void VolumetricFog::apply(Scene& scene) {
  // プリパス:シャドウマップの生成
  {
    // 深度バッファのみのFBOをバインドする
    shadow_fb_.bind(GL_DRAW_FRAMEBUFFER);
    shadow_vp_.apply();

    // 深度バッファをクリアする
    util::clear(1.f);

    //  パイプラインをバインドする
    shadow_prog_.use();
    //util::default_rs().apply();
    util::backface_rs().apply();
    util::default_bs().apply();
    util::depth_test_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);

    // シーンを描画する
    scene.apply(ApplyType::SHADOW);
    scene.draw(DrawType::OPAQUE);
  }

  // パス0:ボリュームのボクセル化
  {
    // パイプラインをバインドする
    p0_prog_.use();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);
    vbuffer_tex_.bind_image(4, GL_WRITE_ONLY, GL_RGBA32F);

    // ディスパッチ
    scene.apply(ApplyType::NO_SHADE);
    glDispatchCompute(constant_.froxel_count[0] / 8, constant_.froxel_count[1] / 8, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // パス1:ボリューメトリックライティングの計算
  {
    // パイプラインをバインドする
    p1_prog_.use();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);
    // shadow_tex_.bind_image(3, GL_READ_ONLY, GL_R32F);
    shadow_tex_.active(3, GL_TEXTURE_2D);
    shadow_ss_.bind(3);
    vbuffer_tex_.bind_image(4, GL_READ_ONLY, GL_RGBA32F);
    lighting_tex_.bind_image(5, GL_WRITE_ONLY, GL_RGBA32F);

    // ディスパッチ
    scene.apply(ApplyType::LIGHT_SHADOW);
    glDispatchCompute(constant_.froxel_count[0] / 8, constant_.froxel_count[1] / 8, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // パス2:シェーディング
  {
    // バックバッファをレンダターゲットにセットする
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    util::screen_viewport().apply();

    // レンダターゲットをクリアする
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);

    // パイプラインをバインドする
    p2_prog_.use();
    util::default_rs().apply();
    util::alpha_blending_bs().apply();
    util::depth_test_dss().apply();

    // リソースをバインドする
    constant_ub_.bind_base(GL_UNIFORM_BUFFER, 15);
    lighting_tex_.active(8, GL_TEXTURE_3D);
    shadow_tex_.active(9, GL_TEXTURE_2D);
    lighting_ss_.bind(8);
    shadow_ss_.bind(9);
    // lighting_tex_.bind_image(5, GL_WRITE_ONLY, GL_RGBA32F);

    // シーンを描画する
    scene.apply(ApplyType::SHADE);
    scene.draw(DrawType::OPAQUE);
  }
}
}  // namespace rtrdemo::tech
