#pragma once

#include <array>
#include <vector>
#include <GL/glew.h>

// OpenGLのRAIIラッパー
namespace garie {
/**
 * @brief GLオブジェクト
 * 
 * DerivedはObject<Derived>を継承し、Object<Derived>からアクセス可能な以下のメソッドを持つ。
 * - `GLuint gen_impl()`:GLオブジェクトを生成する
 * - `void delete_impl(GLuint)`:GLオブジェクトを破棄する
 * 
 * @tparam Derived 派生先の型
 */
template <typename Derived>
class Object {
 public:
  Object() = default;

  Object(const Object&) = delete;

  Object(Object&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
  }

  ~Object() noexcept {
    if (id_) Derived::delete_impl(id_);
  }

  Object& operator=(const Object&) = delete;

  Object& operator=(Object&& other) noexcept {
    if (other.id_ != id_) {
      if (id_) Derived::delete_impl(id_);
      id_ = other.id_;
      other.id_ = 0;
    }
    return *this;
  }

  explicit operator bool() const noexcept {
    return id_ != 0;
  }

  /**
   * @brief 生成する
   */
  void gen() noexcept {
    id_ = Derived::gen_impl();
  }

  /**
   * @brief 破棄する
   */
  void del() noexcept {
    if (id_) {
      Derived::delete_impl(id_);
      id_ = 0;
    }
  }

  /**
   * @brief GLオブジェクトのIDを返す
   * 
   * @return GLuint ID
   */
  GLuint id() const noexcept {
    return id_;
  }

 private:
  GLuint id_ = 0;  ///< GLオブジェクトID
};

/**
 * @brief シェーダオブジェクト
 * 
 * @tparam TYPE シェーダタイプの定数
 */
template <GLenum TYPE>
class Shader : public Object<Shader<TYPE>> {
 public:
  /**
   * @brief コンパイルする
   * 
   * @param string シェーダコードの文字列
   * @param length stringの文字数。-1であれば、stringはヌル終端されている。
   * @return true コンパイルに成功した
   * @return false コンパイルに失敗した
   */
  bool compile(const GLchar* string, GLint length = -1) const noexcept {
    glShaderSource(this->id(), 1, &string, &length);
    glCompileShader(this->id());

    GLint status = GL_FALSE;
    glGetShaderiv(this->id(), GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
  }

  /**
   * @brief SPIR-Vバイナリをロードする
   * 
   * @param binary バイナリへのポインタ
   * @param length binaryのサイズ
   * @return true 成功した
   * @return false 失敗した
   */
  bool load(const void* binary, GLsizei length) const noexcept {
    GLuint id = this->id();
    glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, binary, length);
    glSpecializeShader(this->id(), "main", 0, nullptr, nullptr);

    GLint status = GL_FALSE;
    glGetShaderiv(this->id(), GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
  }

  /**
   * @brief コンパイル時の情報ログを書き出す
   * 
   * @param size ログを書き出す配列の大きさ
   * @param info_log ログを書き出す配列へのポインタ
   * @return GLsizei 実際に書き出したログの文字数
   */
  GLsizei get_info_log(GLsizei size, GLchar* info_log) const noexcept {
    GLsizei length = 0;
    glGetShaderInfoLog(this->id(), size, &length, info_log);
    return length;
  }

 private:
  friend class Object<Shader<TYPE>>;

  static GLuint gen_impl() noexcept {
    return glCreateShader(TYPE);
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteShader(id);
  }
};
using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;
using ComputeShader = Shader<GL_COMPUTE_SHADER>;

/**
 * @brief シェーダプログラム
 * 
 */
class Program : public Object<Program> {
 public:
  /**
   * @brief リンクする
   * 
   * @tparam TYPES シェーダオブジェクトのタイプ
   * @param shaders シェーダオブジェクト
   * @return true リンクに成功した
   * @return false リンクに失敗した
   */
  template <GLenum... TYPES>
  bool link(const Shader<TYPES>&... shaders) const noexcept {
    const GLuint shader_ids[] = {shaders.id()...};
    for (size_t i = 0; i < sizeof...(TYPES); ++i) {
      glAttachShader(id(), shader_ids[i]);
    }
    glLinkProgram(id());

    GLint status = GL_FALSE;
    glGetProgramiv(id(), GL_LINK_STATUS, &status);
    return status == GL_TRUE;
  }

  /**
   * @brief リンク時の情報ログを書き出す
   * 
   * @param size ログを書き出す配列の大きさ
   * @param info_log ログを書き出す配列へのポインタ
   * @return GLsizei 実際に書き出したログの文字数
   */
  GLsizei get_info_log(GLsizei size, GLchar* info_log) const noexcept {
    GLsizei length = 0;
    glGetProgramInfoLog(id(), size, &length, info_log);
    return length;
  }

  void use() const noexcept {
    glUseProgram(id());
  }

  void uniform_block_binding(GLuint index, GLuint binding) const noexcept {
    glUniformBlockBinding(id(), index, binding);
  }

  void shader_storage_block_binding(GLuint index, GLuint binding) const
      noexcept {
    glShaderStorageBlockBinding(id(), index, binding);
  }

 private:
  friend class Object<Program>;

  static GLuint gen_impl() noexcept {
    return glCreateProgram();
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteProgram(id);
  }
};

/**
 * @brief バッファ
 * 
 */
class Buffer : public Object<Buffer> {
 public:
  void bind(GLenum target) const noexcept {
    glBindBuffer(target, id());
  }

  void bind_base(GLenum target, GLuint index) const noexcept {
    glBindBufferBase(target, index, id());
  }

  void bind_range(GLenum target, GLuint index, GLintptr offset,
                  GLsizeiptr size) const noexcept {
    glBindBufferRange(target, index, id(), offset, size);
  }

 private:
  friend class Object<Buffer>;

  static GLuint gen_impl() noexcept {
    GLuint id = 0;
    glGenBuffers(1, &id);
    return id;
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteBuffers(1, &id);
  }
};

/**
 * @brief VAO
 * 
 */
class VertexArray : public Object<VertexArray> {
 public:
  void bind() const noexcept {
    glBindVertexArray(id());
  }

 private:
  friend class Object<VertexArray>;

  static GLuint gen_impl() noexcept {
    GLuint id = 0;
    glGenVertexArrays(1, &id);
    return id;
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteVertexArrays(1, &id);
  }
};

/**
 * @brief VertexArrayを構築するビルダークラス
 * 
 */
class VertexArrayBuilder {
 public:
  /**
   * @brief VAOを生成し、バインドする
   * 
   */
  VertexArrayBuilder() {
    vao_.gen();
    vao_.bind();
  }

  ~VertexArrayBuilder() noexcept = default;

  /**
   * @brief インデックスバッファをバインドする
   * 
   * @param buffer バッファ
   * @return VertexArrayBuilder& 自身を返す
   */
  VertexArrayBuilder& index_buffer(const Buffer& buffer) noexcept {
    buffer.bind(GL_ELEMENT_ARRAY_BUFFER);
    return *this;
  }

  /**
   * @brief 頂点バッファをバインドする
   * 
   * @param buffer バッファ
   * @return VertexArrayBuilder& 自身を返す
   */
  VertexArrayBuilder& vertex_buffer(const Buffer& buffer) noexcept {
    buffer.bind(GL_ARRAY_BUFFER);
    return *this;
  }

  /**
   * @brief 頂点の属性を有効化する
   * 
   * @param index generic vertex attributeの番号
   * @param size 要素数
   * @param type 型のenum
   * @param normalized 正規化されているか
   * @param stride ストライド
   * @param offset オフセット
   * @param divisor インスタンスごとの数
   * @return VertexArrayBuilder& 自身を返す
   */
  VertexArrayBuilder& attribute(GLuint index, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                GLsizeiptr offset, GLuint divisor) noexcept {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, type, normalized, stride,
                          reinterpret_cast<void*>(offset));
    glVertexAttribDivisor(index, divisor);
    return *this;
  }

  /**
   * @brief VAOの状態を確定させる
   * 
   * @return VertexArray VAOを返す
   */
  VertexArray build() noexcept {
    glBindVertexArray(0);
    return std::move(vao_);
  }

 private:
  VertexArray vao_;
};

/**
 * @brief テクスチャ
 * 
 */
class Texture : public Object<Texture> {
 public:
  void bind(GLenum target) const noexcept {
    glBindTexture(target, id());
  }

  void active(GLuint index, GLenum target) const noexcept {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(target, id());
  }

  void bind_image(GLuint unit, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) const noexcept {
    glBindImageTexture(unit, id(), level, layered, layer, access, format);
  }

  void bind_image(GLuint unit, GLint level, GLenum access, GLenum format) const noexcept {
    glBindImageTexture(unit, id(), level, GL_TRUE, 0, access, format);
  }

  void bind_image(GLuint unit, GLenum access, GLenum format) const noexcept {
    glBindImageTexture(unit, id(), 0, GL_TRUE, 0, access, format);
  }

 private:
  friend class Object<Texture>;

  static GLuint gen_impl() noexcept {
    GLuint id = 0;
    glGenTextures(1, &id);
    return id;
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteTextures(1, &id);
  }
};

/**
 * @brief サンプラ
 * 
 */
class Sampler : public Object<Sampler> {
 public:
  void bind(GLuint unit) const noexcept {
    glBindSampler(unit, id());
  }

  void parameter(GLenum pname, GLfloat param) const noexcept {
    glSamplerParameterf(id(), pname, param);
  }

  void parameter(GLenum pname, GLint param) const noexcept {
    glSamplerParameteri(id(), pname, param);
  }

  void parameter(GLenum pname, GLenum param) const noexcept {
    glSamplerParameteri(id(), pname, static_cast<GLint>(param));
  }

  void parameter(GLenum pname, const GLfloat* params) const noexcept {
    glSamplerParameterfv(id(), pname, params);
  }

  void parameter(GLenum pname, const GLint* params) const noexcept {
    glSamplerParameteriv(id(), pname, params);
  }

  void parameter_int(GLenum pname, const GLint* params) const noexcept {
    glSamplerParameterIiv(id(), pname, params);
  }

  void parameter_int(GLenum pname, const GLuint* params) const noexcept {
    glSamplerParameterIuiv(id(), pname, params);
  }

 private:
  friend class Object<Sampler>;

  static GLuint gen_impl() noexcept {
    GLuint id = 0;
    glGenSamplers(1, &id);
    return id;
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteSamplers(1, &id);
  }
};

/**
 * @brief サンプラを構築するビルダークラス
 * 
 */
class SamplerBuilder final {
 public:
  /**
   * @brief サンプラを生成する
   * 
   */
  SamplerBuilder() {
    sampler_.gen();
  }

  ~SamplerBuilder() = default;

  SamplerBuilder& lod_bias(const GLfloat* values) noexcept {
    sampler_.parameter(GL_TEXTURE_LOD_BIAS, values);
    return *this;
  }

  SamplerBuilder& min_filter(GLenum mode) noexcept {
    sampler_.parameter(GL_TEXTURE_MIN_FILTER, mode);
    return *this;
  }

  SamplerBuilder& mag_filter(GLenum mode) noexcept {
    sampler_.parameter(GL_TEXTURE_MAG_FILTER, mode);
    return *this;
  }

  SamplerBuilder& lod(GLfloat min_lod, GLfloat max_lod) noexcept {
    sampler_.parameter(GL_TEXTURE_MIN_LOD, min_lod);
    sampler_.parameter(GL_TEXTURE_MAX_LOD, max_lod);
    return *this;
  }

  SamplerBuilder& min_lod(GLfloat lod) noexcept {
    sampler_.parameter(GL_TEXTURE_MIN_LOD, lod);
    return *this;
  }

  SamplerBuilder& max_lod(GLfloat lod) noexcept {
    sampler_.parameter(GL_TEXTURE_MAX_LOD, lod);
    return *this;
  }

  SamplerBuilder& wrap_s(GLenum mode) noexcept {
    sampler_.parameter(GL_TEXTURE_WRAP_S, mode);
    return *this;
  }

  SamplerBuilder& wrap_t(GLenum mode) noexcept {
    sampler_.parameter(GL_TEXTURE_WRAP_T, mode);
    return *this;
  }

  SamplerBuilder& wrap_r(GLenum mode) noexcept {
    sampler_.parameter(GL_TEXTURE_WRAP_R, mode);
    return *this;
  }

  SamplerBuilder& border_color(const GLfloat* color) noexcept {
    sampler_.parameter(GL_TEXTURE_BORDER_COLOR, color);
    return *this;
  }

  SamplerBuilder& border_color(const GLint* color) noexcept {
    sampler_.parameter(GL_TEXTURE_BORDER_COLOR, color);
    return *this;
  }

  SamplerBuilder& border_color_int(const GLint* color) noexcept {
    sampler_.parameter_int(GL_TEXTURE_BORDER_COLOR, color);
    return *this;
  }

  SamplerBuilder& border_color_int(const GLuint* color) noexcept {
    sampler_.parameter_int(GL_TEXTURE_BORDER_COLOR, color);
    return *this;
  }

  /**
   * @brief 状態を確定させる
   * 
   * @return Sampler サンプラ
   */
  Sampler build() noexcept {
    return std::move(sampler_);
  }

 private:
  Sampler sampler_;
};

/**
 * @brief フレームバッファ
 * 
 */
class Framebuffer final : public Object<Framebuffer> {
 public:
  void bind(GLenum target) const noexcept {
    glBindFramebuffer(target, id());
  }

 private:
  friend class Object<Framebuffer>;

  static GLuint gen_impl() noexcept {
    GLuint id = 0;
    glGenFramebuffers(1, &id);
    return id;
  }

  static void delete_impl(GLuint id) noexcept {
    return glDeleteFramebuffers(1, &id);
  }
};

/**
 * @brief フレームバッファを構築するビルダークラス
 * 
 */
class FramebufferBuilder final {
 public:
  FramebufferBuilder() {
    framebuffer_.gen();
    framebuffer_.bind(GL_FRAMEBUFFER);
  }

  FramebufferBuilder(const FramebufferBuilder&) = delete;

  FramebufferBuilder(FramebufferBuilder&&) = delete;

  ~FramebufferBuilder() = default;

  FramebufferBuilder& operator=(const FramebufferBuilder&) = delete;

  FramebufferBuilder& operator=(FramebufferBuilder&&) = delete;

  FramebufferBuilder& color_texture(GLuint index, const Texture& texture,
                                    GLint level = 0) noexcept {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                         texture.id(), level);
    draw_buffers_.push_back(GL_COLOR_ATTACHMENT0 + index);
    return *this;
  }

  FramebufferBuilder& depth_texture(const Texture& texture,
                                    GLint level = 0) noexcept {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id(),
                         level);
    return *this;
  }

  FramebufferBuilder& depthstencil_texture(const Texture& texture,
                                           GLint level = 0) noexcept {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         texture.id(), level);
    return *this;
  }

  FramebufferBuilder& color_texture_2d(GLuint index, GLenum target,
                                       const Texture& texture,
                                       GLint level = 0) noexcept {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target,
                           texture.id(), level);
    draw_buffers_.push_back(GL_COLOR_ATTACHMENT0 + index);
    return *this;
  }

  FramebufferBuilder& depth_texture_2d(GLenum target, const Texture& texture,
                                       GLint level = 0) noexcept {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target,
                           texture.id(), level);
    return *this;
  }

  FramebufferBuilder& depthstencil_texture_2d(GLenum target,
                                              const Texture& texture,
                                              GLint level = 0) noexcept {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target,
                           texture.id(), level);
    return *this;
  }

  FramebufferBuilder& color_texture_layer(GLuint index, GLenum target,
                                          const Texture& texture,
                                          GLint level = 0,
                                          GLint layer = 0) noexcept {
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                              texture.id(), level, layer);
    draw_buffers_.push_back(GL_COLOR_ATTACHMENT0 + index);
    return *this;
  }

  FramebufferBuilder& depth_texture_layer(GLenum target, const Texture& texture,
                                          GLint level = 0,
                                          GLint layer = 0) noexcept {
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id(),
                              level, layer);
    return *this;
  }

  FramebufferBuilder& depthstencil_texture_layer(GLenum target,
                                                 const Texture& texture,
                                                 GLint level = 0,
                                                 GLint layer = 0) noexcept {
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              texture.id(), level, layer);
    return *this;
  }

  Framebuffer build() noexcept {
    glDrawBuffers(static_cast<GLsizei>(draw_buffers_.size()), draw_buffers_.data());
    const GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return std::move(framebuffer_);
  }

 private:
  Framebuffer framebuffer_;
  std::vector<GLenum> draw_buffers_;
};

/**
 * @brief ラスタライザーステート
 * 
 */
class RasterizationState final {
 public:
  /**
   * @brief 状態を適用する
   * 
   */
  void apply() const noexcept {
    if (is_depth_clamp_enabled_) {
      glEnable(GL_DEPTH_CLAMP);
    } else {
      glDisable(GL_DEPTH_CLAMP);
    }

    if (is_rasterizer_discard_enabled_) {
      glEnable(GL_RASTERIZER_DISCARD);
    } else {
      glDisable(GL_RASTERIZER_DISCARD);
    }

    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode_);
    if (cull_mode_ != GL_NONE) {
      glEnable(GL_CULL_FACE);
      glCullFace(cull_mode_);
    } else {
      glDisable(GL_CULL_FACE);
    }
    glFrontFace(front_face_);
    if (is_depth_bias_enabled_) {
      glEnable(GL_POLYGON_OFFSET_POINT);
      glEnable(GL_POLYGON_OFFSET_LINE);
      glEnable(GL_POLYGON_OFFSET_FILL);
    } else {
      glDisable(GL_POLYGON_OFFSET_POINT);
      glDisable(GL_POLYGON_OFFSET_LINE);
      glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPolygonOffset(depth_bias_constant_, depth_bias_slope_);
    // glPolygonOffsetClamp(depth_bias_constant_, depth_bias_slope_,
    // depth_bias_clamp_);
    glLineWidth(line_width_);
  }

 private:
  friend class RasterizationStateBuilder;

  bool is_depth_clamp_enabled_ = false;
  bool is_rasterizer_discard_enabled_ = false;
  GLenum polygon_mode_ = GL_FILL;
  GLenum cull_mode_ = GL_BACK;
  GLenum front_face_ = GL_CCW;
  bool is_depth_bias_enabled_ = false;
  GLfloat depth_bias_constant_ = 0.f;
  GLfloat depth_bias_clamp_ = 0.f;
  GLfloat depth_bias_slope_ = 0.f;
  GLfloat line_width_ = 1.f;
};

/**
 * @brief ラスタライザーステートを構築するビルダークラス
 * 
 */
class RasterizationStateBuilder final {
 public:
  RasterizationStateBuilder() {}

  RasterizationStateBuilder& enable_depth_clamp() noexcept {
    state_.is_depth_bias_enabled_ = true;
    return *this;
  }

  RasterizationStateBuilder& enable_rasterizer_discard() noexcept {
    state_.is_rasterizer_discard_enabled_ = true;
    return *this;
  }

  RasterizationStateBuilder& polygon_mode(GLenum mode) noexcept {
    state_.polygon_mode_ = mode;
    return *this;
  }

  RasterizationStateBuilder& cull_mode(GLenum mode) noexcept {
    state_.cull_mode_ = mode;
    return *this;
  }

  RasterizationStateBuilder& front_face(GLenum face) noexcept {
    state_.front_face_ = face;
    return *this;
  }

  RasterizationStateBuilder& enable_depth_bias() noexcept {
    state_.is_depth_bias_enabled_ = true;
    return *this;
  }

  RasterizationStateBuilder& depth_bias(GLfloat constant, GLfloat slope,
                                        GLfloat clamp = 0.f) noexcept {
    state_.depth_bias_constant_ = constant;
    state_.depth_bias_slope_ = slope;
    state_.depth_bias_clamp_ = clamp;
    return *this;
  }

  RasterizationState build() noexcept {
    return std::move(state_);
  }

 private:
  RasterizationState state_;
};

/**
 * @brief アタッチメントのブレンドステート
 * 
 */
class ColorBlendAttachmentState final {
 public:
  void apply(GLuint index) const noexcept {
    if (is_enabled_) {
      glEnablei(GL_BLEND, index);
      glBlendFuncSeparatei(index, src_color_, dst_color_, src_alpha_,
                           dst_alpha_);
      glBlendEquationSeparatei(index, color_op_, alpha_op_);
      glColorMaski(index, color_write_mask_[0], color_write_mask_[1],
                   color_write_mask_[2], color_write_mask_[3]);
    } else {
      glDisablei(GL_BLEND, index);
    }
  }

 private:
  friend class ColorBlendStateBuilder;

  bool is_enabled_ = false;
  GLenum src_color_ = GL_ONE;
  GLenum dst_color_ = GL_ZERO;
  GLenum color_op_ = GL_FUNC_ADD;
  GLenum src_alpha_ = GL_ONE;
  GLenum dst_alpha_ = GL_ZERO;
  GLenum alpha_op_ = GL_FUNC_ADD;
  std::array<GLboolean, 4> color_write_mask_{GL_TRUE, GL_TRUE, GL_TRUE,
                                             GL_TRUE};
};

/**
 * @brief ブレンドステート
 * 
 */
class ColorBlendState final {
 public:
  void apply() const noexcept {
    for (GLuint i = 0; i < attachments_.size(); ++i) {
      const auto& attachment = attachments_[i];
      attachment.apply(i);
    }
  }

 private:
  friend class ColorBlendStateBuilder;

  std::array<ColorBlendAttachmentState, 8> attachments_;
};

/**
 * @brief ブレンドステートを構築するビルダークラス
 * 
 */
class ColorBlendStateBuilder final {
 public:
  ColorBlendStateBuilder() {}

  ColorBlendStateBuilder& enable(
      GLuint index, GLenum src_color, GLenum dst_color, GLenum color_op,
      GLenum src_alpha, GLenum dst_alpha, GLenum alpha_op,
      const std::array<GLboolean, 4>& color_write_mask) noexcept {
    auto& attachment = state_.attachments_[index];
    attachment.is_enabled_ = true;
    attachment.src_color_ = src_color;
    attachment.dst_color_ = dst_color;
    attachment.color_op_ = color_op;
    attachment.src_alpha_ = src_alpha;
    attachment.dst_alpha_ = dst_alpha;
    attachment.color_write_mask_ = color_write_mask;
    return *this;
  }

  ColorBlendState build() noexcept {
    return std::move(state_);
  }

 private:
  ColorBlendState state_;
};

/**
 * @brief ステンシル処理のステート
 * 
 */
class StencilOpState final {
 public:
  void apply(GLenum face) const noexcept {
    glStencilOpSeparate(face, fail_op_, depth_fail_op_, pass_op_);
    glStencilFuncSeparate(face, compare_op_, reference_, compare_mask_);
    glStencilMaskSeparate(face, write_mask_);
  }

 private:
  friend class DepthStencilStateBuilder;

  GLenum fail_op_ = GL_KEEP;
  GLenum pass_op_ = GL_KEEP;
  GLenum depth_fail_op_ = GL_KEEP;
  GLenum compare_op_ = GL_ALWAYS;
  GLuint compare_mask_ = 0xffffffff;
  GLuint write_mask_ = 0xffffffff;
  GLint reference_ = 0;
};

/**
 * @brief デプスステンシルステート
 * 
 */
class DepthStencilState final {
 public:
  void apply() const noexcept {
    if (is_depth_test_enabled_) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
    if (is_depth_write_enabled_) {
      glDepthMask(GL_TRUE);
    } else {
      glDepthMask(GL_FALSE);
    }
    glDepthFunc(depth_compare_op_);
    front_.apply(GL_FRONT);
    back_.apply(GL_BACK);
    if (is_depth_bounds_test_enabled_) {
      glDepthRangef(min_depth_bounds_, max_depth_bounds_);
    } else {
      glDepthRangef(0.f, 1.f);
    }
  }

 private:
  friend class DepthStencilStateBuilder;

  bool is_depth_test_enabled_ = false;
  bool is_depth_write_enabled_ = false;
  GLenum depth_compare_op_ = GL_LESS;
  bool is_depth_bounds_test_enabled_ = false;
  bool is_stencil_test_enabled_ = false;
  StencilOpState front_;
  StencilOpState back_;
  GLfloat min_depth_bounds_ = 0.f;
  GLfloat max_depth_bounds_ = 1.f;
};

/**
 * @brief デプスステンシルステートを構築するビルダークラス
 * 
 */
class DepthStencilStateBuilder final {
 public:
  DepthStencilStateBuilder() {}

  DepthStencilStateBuilder& enable_depth_test(GLenum compare_op) noexcept {
    state_.is_depth_test_enabled_ = true;
    state_.depth_compare_op_ = compare_op;
    return *this;
  }

  DepthStencilStateBuilder& enable_depth_write() noexcept {
    state_.is_depth_write_enabled_ = true;
    return *this;
  }

  DepthStencilStateBuilder& enable_stencil_test() noexcept {
    state_.is_stencil_test_enabled_ = true;
    return *this;
  }

  DepthStencilStateBuilder& front_stencil_op(GLenum fail_op, GLenum pass_op,
                                             GLenum depth_fail_op,
                                             GLenum compare_op,
                                             GLuint compare_mask = 0xffffffff,
                                             GLuint write_mask = 0xffffffff,
                                             GLint reference = 0) noexcept {
    state_.front_.fail_op_ = fail_op;
    state_.front_.pass_op_ = pass_op;
    state_.front_.depth_fail_op_ = fail_op;
    state_.front_.compare_op_ = compare_op;
    state_.front_.compare_mask_ = compare_mask;
    state_.front_.write_mask_ = write_mask;
    state_.front_.reference_ = reference;
    return *this;
  }

  DepthStencilStateBuilder& back_stencil_op(GLenum fail_op, GLenum pass_op,
                                            GLenum depth_fail_op,
                                            GLenum compare_op,
                                            GLuint compare_mask = 0xffffffff,
                                            GLuint write_mask = 0xffffffff,
                                            GLint reference = 0) noexcept {
    state_.back_.fail_op_ = fail_op;
    state_.back_.pass_op_ = pass_op;
    state_.back_.depth_fail_op_ = fail_op;
    state_.back_.compare_op_ = compare_op;
    state_.back_.compare_mask_ = compare_mask;
    state_.back_.write_mask_ = write_mask;
    state_.back_.reference_ = reference;
    return *this;
  }

  DepthStencilStateBuilder& enable_depth_bounds_test(
      GLfloat min_bounds, GLfloat max_bounds) noexcept {
    state_.is_depth_bounds_test_enabled_ = true;
    state_.min_depth_bounds_ = min_bounds;
    state_.max_depth_bounds_ = max_bounds;
    return *this;
  }

  DepthStencilState build() noexcept {
    return std::move(state_);
  }

 private:
  DepthStencilState state_;
};

class Viewport {
public:
  Viewport() = default;

  Viewport(float x, float y, float w, float h) noexcept
    : v{x, y, w, h} {}

  Viewport(std::array<float, 4> v) noexcept
    : v(v) {}

  void apply(GLuint index = 0) const noexcept {
    glViewportIndexedfv(index, v.data());
  }

  union {
    struct {
      float x;
      float y;
      float width;
      float height;
    };
    std::array<float, 4> v;
  };
};

class Scissor {
public:
  Scissor() = default;

  Scissor(GLint l, GLint b, GLint w, GLint h) noexcept
    : v{l, b, w, h} {}

  Scissor(std::array<GLint, 4> v) noexcept
    : v(v) {}

  void apply(GLuint index = 0) const noexcept {
    glScissorIndexedv(index, v.data());
  }

  union {
    struct {
      GLint left;
      GLint bottom;
      GLint width;
      GLint height;
    };
    std::array<GLint, 4> v;
  };
};
}  // namespace garie
