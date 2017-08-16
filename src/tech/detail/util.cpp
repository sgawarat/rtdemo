#include <rtdemo/tech/detail/util.hpp>

namespace rtdemo {
namespace tech {
namespace detail {
template <GLenum TYPE>
garie::Shader<TYPE> compile_shader_from_file(const char* path,
                                             std::string* log_ptr) {
  garie::Shader<TYPE> shader;

  std::ifstream ifs(path);
  if (!ifs) {
    if (log_ptr) *log_ptr = fmt::format("Cannot open {}", path);
    RT_LOG(error, "failed to open (path:{})", path);
    return garie::Shader<TYPE>{};
  }

  // calc file size
  ifs.seekg(0, std::ios::end);
  const auto end = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  const auto length = end - ifs.tellg();

  static std::vector<char> code;
  code.resize(length);
  ifs.read(code.data(), code.size());

  shader.gen();
  if (!shader.compile(code.data(), code.size())) {
    static GLchar info_log[1024];
    shader.get_info_log(1024, info_log);
    if (log_ptr) *log_ptr = info_log;
    RT_LOG(error, "Failed to compile shader (type:{}, info_log:{})", TYPE,
           info_log);
    return garie::Shader<TYPE>{};
  }

  return shader;
}

// explicit instantiation
template
garie::VertexShader compile_shader_from_file(const char*, std::string*);
template
garie::FragmentShader compile_shader_from_file(const char*, std::string*);

garie::Program link_program(const garie::VertexShader& vert,
                            const garie::FragmentShader& frag,
                            std::string* log_ptr) {
  garie::Program prog;
  prog.gen();
  if (!prog.link(vert, frag)) {
    static GLchar info_log[1024];
    prog.get_info_log(1024, info_log);
    if (log_ptr) *log_ptr = info_log;
    RT_LOG(error, "Failed to link shader program (info_log:{})", info_log);
    return garie::Program();
  }
  return prog;
}

// ScreenQuad geometry for common use
const garie::VertexArray& screen_quad_vao() {
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

void draw_screen_quad() {
  glEnable(GL_SCISSOR_TEST);
  glScissor(0, 0, 1280, 720);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

const garie::RasterizationState& default_rs() {
  static garie::RasterizationState rs;
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
}  // namespace detail
}  // namespace tech
}  // namespace rtdemo
