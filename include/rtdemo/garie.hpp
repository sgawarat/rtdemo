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
        if (id_) Derived::destroy_impl(id_);
    }

    Object& operator=(const Object&) = delete;

    Object& operator=(Object&& other) noexcept {
        if (&other != this) {
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    explicit operator bool() const noexcept {
        return id_ != 0;
    }

    void create() noexcept {
        id_ = Derived::create_impl();
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

    static GLuint create_impl() noexcept {
        return glCreateShader(TYPE);
    }

    static void destroy_impl(GLuint id) noexcept {
        return glDeleteShader(id);
    }
};
using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;

class Program : public Object<Program> {
public:
    // template <GLenum TYPE>
    // void attach(const Shader<TYPE>& shader) const noexcept {
    //     glAttachShader(id(), shader.id());
    // }

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

private:
    friend class Object<Program>;

    static GLuint create_impl() noexcept {
        return glCreateProgram();
    }

    static void destroy_impl(GLuint id) noexcept {
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

    static GLuint create_impl() noexcept {
        GLuint id = 0;
        glGenBuffers(1, &id);
        return id;
    }

    static void destroy_impl(GLuint id) noexcept {
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

    static GLuint create_impl() noexcept {
        GLuint id = 0;
        glGenVertexArrays(1, &id);
        return id;
    }

    static void destroy_impl(GLuint id) noexcept {
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
}  // namespace garie
