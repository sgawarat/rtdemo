#pragma once

#include <rtdemo/garie.hpp>

namespace rtdemo {
namespace scene {
class StaticScene {
public:
    struct Vertex {
        float position[3];
    };
    using Index = uint16_t;

    static constexpr GLint IN_POSITION_LOC = 0;

    bool init();

    void apply() const noexcept;

    void draw() const noexcept;

private:
    garie::VertexArray vao_;
    garie::Buffer vbo_;
    garie::Buffer ibo_;
    GLsizei count_ = 0;
};
}  // namespace scene
}  // namespace rtdemo
