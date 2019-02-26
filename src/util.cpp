#include <rtdemo/util.hpp>
#include <fstream>
#include <vector>
#include <rtdemo/logging.hpp>
#include <rtdemo/application.hpp>

namespace rtdemo::util {
namespace {
template <GLenum TYPE>
inline garie::Shader<TYPE> compile_shader_from_file(const char* path,
                                             std::string* log_ptr) {
  garie::Shader<TYPE> shader;

  // ファイルを開く
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (!ifs) {
    if (log_ptr) *log_ptr = fmt::format("{}を開けない", path);
    RT_ERROR("ファイルのオープンに失敗した (path:{})", path);
    return garie::Shader<TYPE>{};
  }

  // ファイルサイズを計算する
  const auto end = ifs.seekg(0, std::ios::end).tellg();
  const auto beg = ifs.seekg(0, std::ios::beg).tellg();
  const auto length = end - beg;

  // ファイルから読み込む
  static std::vector<char> code;
  code.resize(length);
  ifs.read(code.data(), code.size());

  // シェーダを生成する
  shader.gen();
  if (!shader.compile(code.data(), static_cast<GLint>(code.size()))) {
    static GLchar info_log[1024];
    shader.get_info_log(1024, info_log);
    if (log_ptr) *log_ptr = info_log;
    RT_ERROR("シェーダのコンパイルに失敗した (type:{}, info_log:{})", TYPE,
           info_log);
    return garie::Shader<TYPE>{};
  }

  return shader;
}

template <GLenum TYPE>
inline garie::Shader<TYPE> load_shader_from_file(const char* path,
                                             std::string* log_ptr) {
  garie::Shader<TYPE> shader;

  // ファイルを開く
  std::ifstream ifs(path, std::ios::in | std::ios::binary);
  if (!ifs) {
    if (log_ptr) *log_ptr = fmt::format("{}を開けない", path);
    RT_ERROR("ファイルのオープンに失敗した (path:{})", path);
    return garie::Shader<TYPE>{};
  }

  // ファイルサイズを計算する
  const auto end = ifs.seekg(0, std::ios::end).tellg();
  const auto beg = ifs.seekg(0, std::ios::beg).tellg();
  const auto length = end - beg;

  // ファイルから読み込む
  static std::vector<char> binary;
  binary.resize(length);
  ifs.read(binary.data(), binary.size());

  // シェーダを生成する
  shader.gen();
  if (!shader.load(binary.data(), static_cast<GLsizei>(binary.size()))) {
    static GLchar info_log[1024];
    shader.get_info_log(1024, info_log);
    if (log_ptr) *log_ptr = info_log;
    RT_ERROR("シェーダのコンパイルに失敗した (type:{}, info_log:{})", TYPE,
           info_log);
    return garie::Shader<TYPE>{};
  }

  return shader;
}

template <GLenum... TYPES>
inline garie::Program link_shader_program(const garie::Shader<TYPES>&... shaders,
                                   std::string* log_ptr) {
  garie::Program prog;
  prog.gen();
  if (!prog.link(shaders...)) {
    static GLchar info_log[1024];
    prog.get_info_log(1024, info_log);
    if (log_ptr) *log_ptr = info_log;
    RT_ERROR("シェーダプログラムのリンクに失敗した (info_log:{})", info_log);
    return garie::Program();
  }
  return prog;
}
}  // namespace

garie::VertexShader compile_vertex_shader_from_file(const char* path, std::string* log_ptr) {
  return compile_shader_from_file<GL_VERTEX_SHADER>(path, log_ptr);
}

garie::FragmentShader compile_fragment_shader_from_file(const char* path, std::string* log_ptr) {
  return compile_shader_from_file<GL_FRAGMENT_SHADER>(path, log_ptr);
}

garie::ComputeShader compile_compute_shader_from_file(const char* path, std::string* log_ptr) {
  return compile_shader_from_file<GL_COMPUTE_SHADER>(path, log_ptr);
}

garie::VertexShader load_vertex_shader_from_file(const char* path, std::string* log_ptr) {
  return load_shader_from_file<GL_VERTEX_SHADER>(path, log_ptr);
}

garie::FragmentShader load_fragment_shader_from_file(const char* path, std::string* log_ptr) {
  return load_shader_from_file<GL_FRAGMENT_SHADER>(path, log_ptr);
}

garie::ComputeShader load_compute_shader_from_file(const char* path, std::string* log_ptr) {
  return load_shader_from_file<GL_COMPUTE_SHADER>(path, log_ptr);
}

garie::Program link_program(const garie::VertexShader& vert, const garie::FragmentShader& frag, std::string* log_ptr) {
  return link_shader_program<GL_VERTEX_SHADER, GL_FRAGMENT_SHADER>(vert, frag, log_ptr);
}

garie::Program link_program(const garie::VertexShader& vert, std::string* log_ptr) {
  return link_shader_program<GL_VERTEX_SHADER>(vert, log_ptr);
}

garie::Program link_program(const garie::ComputeShader& comp, std::string* log_ptr) {
  return link_shader_program<GL_COMPUTE_SHADER>(comp, log_ptr);
}

const garie::VertexArray& screen_quad_vao() {
  static garie::VertexArray vao_;
  static garie::Buffer vbo_;
  if (!vao_) {
    garie::Buffer vbo;
    vbo.gen();
    vbo.bind(GL_ARRAY_BUFFER);
    const float vertices[] = {
        1.f, 1.f,
        -1.f, -1.f,
        1.f, -1.f,
        -1.f, -1.f,
        1.f, 1.f,
        -1.f, 1.f,
    };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    garie::VertexArray vao = garie::VertexArrayBuilder()
                                 .vertex_buffer(vbo)
                                 .attribute(0, 2, GL_FLOAT, GL_FALSE, 8, 0, 0)
                                 .build();

    vao_ = std::move(vao);
    vbo_ = std::move(vbo);
  }
  return vao_;
}

void draw_screen_quad() {
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

const garie::VertexArray& screen_triangle_vao() {
  static garie::VertexArray vao_;
  static garie::Buffer vbo_;
  if (!vao_) {
    garie::Buffer vbo;
    vbo.gen();
    vbo.bind(GL_ARRAY_BUFFER);
    const float vertices[] = {
        -1.f, -1.f, 3.f, -1.f, -1.f, 3.f,
    };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    garie::VertexArray vao = garie::VertexArrayBuilder()
                                 .vertex_buffer(vbo)
                                 .attribute(0, 2, GL_FLOAT, GL_FALSE, 8, 0, 0)
                                 .build();

    vao_ = std::move(vao);
    vbo_ = std::move(vbo);
  }
  return vao_;
}

void draw_screen_triangle() {
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

const garie::RasterizationState& default_rs() {
  static garie::RasterizationState rs;
  return rs;
}

const garie::RasterizationState& discard_rs() {
  static garie::RasterizationState rs = garie::RasterizationStateBuilder()
      .enable_rasterizer_discard()
      .build();
  return rs;
}

const garie::ColorBlendState& default_bs() {
  static garie::ColorBlendState bs;
  return bs;
}

const garie::ColorBlendState& alpha_blending_bs() {
  static garie::ColorBlendState bs =
      garie::ColorBlendStateBuilder()
          .enable(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD,
                  GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD,
                  {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE})
          .build();
  return bs;
}

const garie::ColorBlendState& additive_bs() {
  static garie::ColorBlendState bs =
      garie::ColorBlendStateBuilder()
          .enable(0, GL_ONE, GL_ONE, GL_FUNC_ADD, GL_ONE, GL_ONE, GL_FUNC_ADD,
                  {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE})
          .build();
  return bs;
}

const garie::DepthStencilState& default_dss() {
  static garie::DepthStencilState dss;
  return dss;
}

const garie::DepthStencilState& depth_test_dss() {
  static garie::DepthStencilState dss =
      garie::DepthStencilStateBuilder()
          .enable_depth_test(GL_LESS)
          .enable_depth_write()
          .enable_depth_bounds_test(0.f, 1.f)
          .build();
  return dss;
}

const garie::DepthStencilState& depth_test_no_write_dss() {
  static garie::DepthStencilState dss =
      garie::DepthStencilStateBuilder()
          .enable_depth_test(GL_LEQUAL)
          .enable_depth_bounds_test(0.f, 1.f)
          .build();
  return dss;
}

void clear(std::array<float, 4> color) {
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
}

void clear(float depth) {
  glDepthMask(GL_TRUE);
  glClearDepthf(depth);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void clear(std::array<float, 4> color, float depth) {
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClearDepthf(depth);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void clear(std::array<float, 4> color, float depth, GLint stencil) {
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilMask(GL_TRUE);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClearDepthf(depth);
  glClearStencil(stencil);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

garie::Viewport screen_viewport() {
  auto& app = Application::get();
  return garie::Viewport(0.f, 0.f, static_cast<float>(app.screen_width()), static_cast<float>(app.screen_height()));
}

garie::Scissor screen_scissor() {
  auto& app = Application::get();
  return garie::Scissor(0, 0, app.screen_width(), app.screen_height());
}
}  // namespace rtdemo::util
