#pragma once

#include <rtdemo/scene/scene.hpp>

namespace rtdemo::tech {
/**
 * @brief テクニックのインターフェイス
 * 
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
   * 
   */
  virtual void update() = 0;

  /**
   * @brief GUIを更新する
   * 
   */
  virtual void update_gui() = 0;

  /**
   * @brief シーンにテクニックを適用して描画する
   * 
   * @param scene 描画するシーン
   */
  virtual void apply(scene::Scene& scene) = 0;
};
}  // namespace rtdemo::tech
