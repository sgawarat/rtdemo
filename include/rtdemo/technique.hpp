#pragma once

#include "application.hpp"
#include "scene.hpp"

namespace rtdemo {
/**
 * @brief テクニックのインターフェイス
 */
class Technique {
 public:
  virtual ~Technique() noexcept {}

  /**
   * @brief リソースを用意する
   * 
   * @return true 成功した
   * @return false 失敗した
   */
  virtual bool restore() = 0;

  /**
   * @brief リソースを破棄する
   * 
   * @return true 成功した
   * @return false 失敗した
   */
  virtual bool invalidate() = 0;

  /**
   * @brief 状態を更新する
   */
  virtual void update() = 0;

  /**
   * @brief GUIを更新する
   */
  virtual void update_gui() = 0;

  /**
   * @brief シーンにテクニックを適用して描画する
   * 
   * @param scene 描画するシーン
   */
  virtual void apply(Scene& scene) = 0;
};

/**
 * @brief staticなテクニックを定義するマクロ
 * 
 * ソースファイルでrtdemo::tech下に記述すると、プログラム起動時にT型のテクニックを登録してくれる。
 */
#define RT_MANAGED_TECHNIQUE(T) \
  namespace { \
    static struct ManagedTechnique_##T { \
      ManagedTechnique_##T() { \
        ::rtdemo::Application::get().insert_technique(#T, std::make_shared<T>()); \
      } \
    } managed_technique_##T##_; \
  }
}  // namespace rtdemo
