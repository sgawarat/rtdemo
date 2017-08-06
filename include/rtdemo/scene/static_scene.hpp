#pragma once

#include <vector>
#include <cstdint>
#include <rtdemo/garie.hpp>

namespace rtdemo {
namespace scene {
class StaticScene {
public:
    struct Vertex {
        float position[3];
    };
    using Index = uint16_t;

    struct Material {
        float diffuse[3];
        float _pad[61];
    };
    static_assert(sizeof(Material) % 256 == 0, "Material requires UBO alignment");

    static constexpr GLint IN_POSITION_LOC = 0;

    bool init();

    void apply() const noexcept;

    void draw() const noexcept;

private:
    struct DrawParam {
        size_t material_index;
        GLsizei index_count;
        GLsizeiptr index_offset;
        GLint base_vertex;
    };

    garie::VertexArray vao_;
    garie::Buffer vbo_;
    garie::Buffer ibo_;
    garie::Buffer material_ubo_;
    std::vector<DrawParam> draw_params_;
};
}  // namespace scene
}  // namespace rtdemo
