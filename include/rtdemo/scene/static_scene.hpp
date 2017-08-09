#pragma once

#include <vector>
#include <rtdemo/garie.hpp>
#include <rtdemo/layout/static_layout.hpp>
#include <rtdemo/scene/scene.hpp>

namespace rtdemo {
namespace scene {
class StaticScene : public Scene {
public:
    using Layout = layout::StaticLayout;

    ~StaticScene() noexcept override {}

    bool restore() override;

    bool invalidate() override;

    void update() override;

    void update_gui() override;

    void draw(size_t index) override;

private:
    enum class DrawMode : int {
        DRAW,
        DRAW_INDIRECT,
    };

    struct Command {
        GLuint index_count;
        GLuint instance_count;
        GLuint index_first;
        GLuint base_vertex;
        GLuint base_instance;
    };

    Layout::Camera* mapped_camera_ubo_ = nullptr;
    float camera_distance_ = 0.f;
    float camera_yaw_ = 0.f;
    float camera_pitch_ = 0.f;
    DrawMode draw_mode_ = DrawMode::DRAW;

    garie::VertexArray vao_;
    garie::Buffer vbo_;
    garie::Buffer ibo_;
    garie::Buffer camera_ubo_;
    garie::Buffer resource_index_ssbo_;
    garie::Buffer material_ssbo_;
    garie::Buffer light_ssbo_;
    garie::Buffer dio_;
    std::vector<Command> commands_;
};
}  // namespace scene
}  // namespace rtdemo
