#include <rtdemo/tech/detail/util.hpp>

namespace rtdemo {
namespace tech {
namespace detail {
// ScreenQuad geometry for common use
const garie::VertexArray& screen_quad() {
    static garie::VertexArray vao_;
    static garie::Buffer vbo_;
    if (!vao_) {
        garie::Buffer vbo;
        vbo.gen();
        vbo.bind(GL_ARRAY_BUFFER);
        const float vertices[] = {
            -1.f, -1.f,
            -1.f, 3.f,
            3.f, -1.f,
        };
        glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

        garie::VertexArray vao;
        vao.gen();
        garie::VertexArrayBuilder(vao)
            .vertex_buffer(vbo)
            .attribute(0, 2, GL_FLOAT, GL_FALSE, 8, 0, 0)
            .build();

        vao_ = std::move(vao);
        vbo_ = std::move(vbo);
    }
    return vao_;
}

void draw_screen_quad() {
    screen_quad().bind();
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1280, 720);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
}  // namespace detail
}  // namespace tech
}  // namespace rtdemo
