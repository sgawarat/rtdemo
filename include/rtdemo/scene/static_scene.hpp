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
        float normal[3];
    };
    using Index = uint16_t;

    struct Material {
        float ambient[3];
        float _ambient;
        float diffuse[3];
        float _diffuse;
        float specular[3];
        float specular_power;
    };

    struct Light {
        float position_w[3];
        float _position_w;
    };

    enum class DrawMode {
        DRAW,
        DRAW_INDIRECT,
        // MULTIDRAW,
        // MULTIDRAW_INDIRECT,
    };

    static constexpr GLint IN_POSITION_LOC = 0;
    static constexpr GLint IN_NORMAL_LOC = 1;

    bool init();

    void apply() const noexcept;

    void draw() const noexcept;

    void set_draw_mode(DrawMode mode) noexcept {
        draw_mode_ = mode;
    }

private:
    struct ResourceIndex {
        uint32_t material_index;
    };
    struct Command {
        GLuint index_count;
        GLuint instance_count;
        GLuint index_first;
        GLuint base_vertex;
        GLuint base_instance;
    };

    garie::VertexArray vao_;
    garie::Buffer vbo_;
    garie::Buffer ibo_;
    garie::Buffer resource_indices_ssbo_;
    garie::Buffer materials_ssbo_;
    garie::Buffer lights_ssbo_;
    DrawMode draw_mode_ = DrawMode::DRAW;
    garie::Buffer dio_;
    std::vector<Command> commands_;
};
}  // namespace scene
}  // namespace rtdemo
