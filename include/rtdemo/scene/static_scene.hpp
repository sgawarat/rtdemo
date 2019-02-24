#pragma once

#include <vector>
#include <rtdemo/garie.hpp>
#include <rtdemo/scene.hpp>

namespace rtdemo::scene {
/**
 * @brief 静的なシーン
 */
class StaticScene final : public Scene {
 public:
  ~StaticScene() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(ApplyType type) override;

  void draw(DrawType type) override;

 private:
  /**
   * @brief 描画モード
   */
  enum class DrawMode : int {
    DRAW,  ///< 従来の描画
    DRAW_INDIRECT,  ///< Indirect描画
  };

  /**
   * @brief 間接描画コマンド
   */
  struct Command {
    GLuint index_count;
    GLuint instance_count;
    GLuint index_first;
    GLuint base_vertex;
    GLuint base_instance;
  };

  float camera_center_ = 0.f;  ///< カメラの中心
  float camera_distance_ = 0.f;  ///< カメラの距離
  float camera_yaw_ = 0.f;  ///< カメラのY軸回転角度
  float camera_pitch_ = 0.f;  ///< カメラのX軸回転角度
  float lens_depth_ = 100.f;  ///< ファー面の距離
  DrawMode draw_mode_ = DrawMode::DRAW;  ///< 描画モード

  garie::VertexArray vao_;
  garie::Buffer vbo_;
  garie::Buffer ibo_;
  garie::Buffer camera_ubo_;
  garie::Buffer resource_index_ssbo_;
  garie::Buffer material_ssbo_;
  garie::Buffer light_ssbo_;
  garie::Buffer shadow_ssbo_;
  size_t light_count_ = 0;
  garie::Buffer dio_;  ///< indirect描画コマンドのバッファ
  std::vector<Command> commands_;
};
}  // namespace rtdemo::scene
