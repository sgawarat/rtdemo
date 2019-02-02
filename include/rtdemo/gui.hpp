#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>
#include "garie.hpp"

namespace rtdemo::gui {
class Gui final {
 public:
  /**
  * @brief 初期化する。
  * 
  * @param window 対象のウィンドウハンドル
  * @return true 初期化に成功した。
  * @return false 初期化に失敗した。
  */
  bool init(GLFWwindow* window);

  /**
   * @brief 破棄する。
   * 
   */
  void terminate();

  /**
   * @brief 新しいフレームを開始する。
   * 
   */
  void new_frame();

  /**
   * @brief 描画する。
   * 
   */
  void render();

  /**
   * @brief マウスのボタンが押されたときに呼び出すメソッド
   * 
   * @param button ボタン番号
   * @param action ボタンの状態
   * @param mods 
   */
  void on_mouse_button(int button, int action, int mods);

  /**
   * @brief マウスホイールが回されたときに呼び出すメソッド
   * 
   * @param x 横方向の速度
   * @param y 右方向の速度
   */
  void on_scroll(double x, double y);

  /**
   * @brief キーが押されたときに呼び出すメソッド
   * 
   * @param key キーのコード
   * @param scancode スキャンコード
   * @param action キーの状態
   * @param mods 
   */
  void on_key(int key, int scancode, int action, int mods);

  /**
   * @brief 文字が送られたときに呼び出すメソッド
   * 
   * @param c ユニコード文字
   */
  void on_char(unsigned int c);

 private:
  GLFWwindow* window_ = nullptr;
  double time_ = 0.0;
  bool mouse_pressed_[3] = {};
  garie::Program prog_;
  garie::RasterizationState rs_;
  garie::ColorBlendState cbs_;
  garie::DepthStencilState dss_;
  garie::VertexArray va_;
  garie::Buffer ib_;
  garie::Buffer vb_;
  garie::Texture font_tex_;
  garie::Sampler font_ss_;
};
}  // namespace rtdemo::gui
