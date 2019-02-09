#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include "technique.hpp"

namespace rtdemo::tech {
/**
 * @brief Forward Shading
 * 
 */
class ForwardShading final : public Technique {
 public:
  ~ForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(scene::Scene& scene) override;

 private:
  /**
   * @brief デバッグ表示
   * 
   */
  enum class DebugView : int {
    DEFAULT,  ///< 通常
    POSITION,  ///< 位置
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
  };

  garie::Program prog_;
  DebugView debug_view_ = DebugView::DEFAULT;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
