#pragma once

#include <GL/glew.h>

// OpenGL RAII wrappers
namespace garie {
template <typename Derived>
class Object {
public:
    Object() = default;

    Object(const Object&) = delete;

    Object(Object&& other) noexcept
      : id_(other.id_) {
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

    void gen() noexcept {
        id_ = Derived::gen_impl();
    }

    GLuint id() const noexcept {
        return id_;
    }

private:
    GLuint id_ = 0;
};

template <GLenum TYPE>
class Shader : public Object<Shader<TYPE>> {
public:
    bool compile(const GLchar* string, GLint length = -1) const noexcept {
        glShaderSource(this->id(), 1, &string, &length);
        glCompileShader(this->id());

        GLint status = GL_FALSE;
        glGetShaderiv(this->id(), GL_COMPILE_STATUS, &status);
        return status == GL_TRUE;
    }

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

class Program : public Object<Program> {
public:
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

    void shader_storage_block_binding(GLuint index, GLuint binding) const noexcept {
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

class Buffer : public Object<Buffer> {
public:
    void bind(GLenum target) const noexcept {
        glBindBuffer(target, id());
    }

    void bind_base(GLenum target, GLuint index) const noexcept {
        glBindBufferBase(target, index, id());
    }

    void bind_range(GLenum target, GLuint index, GLintptr offset, GLsizeiptr size) const noexcept {
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

class VertexArrayBuilder {
public:
    VertexArrayBuilder(const VertexArray& vao) {
        vao.bind();
    }

    VertexArrayBuilder(const VertexArrayBuilder&) = delete;

    VertexArrayBuilder(VertexArrayBuilder&&) = delete;

    ~VertexArrayBuilder() noexcept = default;

    VertexArrayBuilder& operator=(const VertexArrayBuilder&) = delete;

    VertexArrayBuilder& operator=(VertexArrayBuilder&&) = delete;

    const VertexArrayBuilder& index_buffer(const Buffer& buffer) const noexcept {
        buffer.bind(GL_ELEMENT_ARRAY_BUFFER);
        return *this;
    }

    const VertexArrayBuilder& vertex_buffer(const Buffer& buffer) const noexcept {
        buffer.bind(GL_ARRAY_BUFFER);
        return *this;
    }

    const VertexArrayBuilder& attribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizeiptr offset, GLuint divisor) const noexcept {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, normalized, stride, (const void*)offset);
        glVertexAttribDivisor(index, divisor);
        return *this;
    }

    void build() const noexcept {
        glBindVertexArray(0);
    }
};

class Texture : public Object<Texture> {
public:
    void bind(GLenum target) const noexcept {
        glBindTexture(target, id());
    }

    void active(GLuint index, GLenum target) const noexcept {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(target, id());
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

class SamplerBuilder final {
public:
    SamplerBuilder(const Sampler& sampler) : sampler_(sampler) {}

    SamplerBuilder(const SamplerBuilder&) = delete;

    SamplerBuilder(SamplerBuilder&&) = delete;
    
    ~SamplerBuilder() = default;

    SamplerBuilder& operator=(const SamplerBuilder&) = delete;
    
    SamplerBuilder& operator=(SamplerBuilder&&) = delete;
    
    const SamplerBuilder& lod_bias(const GLfloat* values) const noexcept {
        sampler_.parameter(GL_TEXTURE_LOD_BIAS, values);
        return *this;
    }

    const SamplerBuilder& min_filter(GLenum mode) const noexcept {
        sampler_.parameter(GL_TEXTURE_MIN_FILTER, mode);
        return *this;
    }

    const SamplerBuilder& mag_filter(GLenum mode) const noexcept {
        sampler_.parameter(GL_TEXTURE_MAG_FILTER, mode);
        return *this;
    }

    const SamplerBuilder& lod(GLfloat min_lod, GLfloat max_lod) const noexcept {
        sampler_.parameter(GL_TEXTURE_MIN_LOD, min_lod);
        sampler_.parameter(GL_TEXTURE_MAX_LOD, max_lod);
        return *this;
    }

    const SamplerBuilder& min_lod(GLfloat lod) const noexcept {
        sampler_.parameter(GL_TEXTURE_MIN_LOD, lod);
        return *this;
    }

    const SamplerBuilder& max_lod(GLfloat lod) const noexcept {
        sampler_.parameter(GL_TEXTURE_MAX_LOD, lod);
        return *this;
    }

    const SamplerBuilder& wrap_s(GLenum mode) const noexcept {
        sampler_.parameter(GL_TEXTURE_WRAP_S, mode);
        return *this;
    }

    const SamplerBuilder& wrap_t(GLenum mode) const noexcept {
        sampler_.parameter(GL_TEXTURE_WRAP_T, mode);
        return *this;
    }

    const SamplerBuilder& wrap_r(GLenum mode) const noexcept {
        sampler_.parameter(GL_TEXTURE_WRAP_R, mode);
        return *this;
    }

    const SamplerBuilder& border_color(const GLfloat* color) const noexcept {
        sampler_.parameter(GL_TEXTURE_BORDER_COLOR, color);
        return *this;
    }

    const SamplerBuilder& border_color(const GLint* color) const noexcept {
        sampler_.parameter(GL_TEXTURE_BORDER_COLOR, color);
        return *this;
    }

    const SamplerBuilder& border_color_int(const GLint* color) const noexcept {
        sampler_.parameter_int(GL_TEXTURE_BORDER_COLOR, color);
        return *this;
    }

    const SamplerBuilder& border_color_int(const GLuint* color) const noexcept {
        sampler_.parameter_int(GL_TEXTURE_BORDER_COLOR, color);
        return *this;
    }

    void build() const noexcept {}

private:
    const Sampler& sampler_;
};

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

class FramebufferBuilder final {
public:
    FramebufferBuilder(const Framebuffer& framebuffer) {
        framebuffer.bind(GL_FRAMEBUFFER);
    }

    FramebufferBuilder(const FramebufferBuilder&) = delete;

    FramebufferBuilder(FramebufferBuilder&&) = delete;

    ~FramebufferBuilder() = default;

    FramebufferBuilder& operator=(const FramebufferBuilder&) = delete;

    FramebufferBuilder& operator=(FramebufferBuilder&&) = delete;

    const FramebufferBuilder& color_texture(GLuint index, const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& depth_texture(const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& depthstencil_texture(const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& color_texture_2d(GLuint index, GLenum target, const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& depth_texture_2d(GLenum target, const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& depthstencil_texture_2d(GLenum target, const Texture& texture, GLint level = 0) const noexcept {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, texture.id(), level);
        return *this;
    }

    const FramebufferBuilder& color_texture_layer(GLuint index, GLenum target, const Texture& texture, GLint level = 0, GLint layer = 0) const noexcept {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture.id(), level, layer);
        return *this;
    }

    const FramebufferBuilder& depth_texture_layer(GLenum target, const Texture& texture, GLint level = 0, GLint layer = 0) const noexcept {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture.id(), level, layer);
        return *this;
    }

    const FramebufferBuilder& depthstencil_texture_layer(GLenum target, const Texture& texture, GLint level = 0, GLint layer = 0) const noexcept {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), level, layer);
        return *this;
    }

    bool build() const noexcept {
        const GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return result == GL_FRAMEBUFFER_COMPLETE;
    }
};
}  // namespace garie
