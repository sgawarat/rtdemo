#pragma once

#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/technique.hpp>

namespace rtdemo::tech {
/**
 * @brief Forward Shading
 */
class ForwardShading final : public Technique {
 public:
  ~ForwardShading() noexcept override {}

  bool restore() override;

  bool invalidate() override;

  void update() override;

  void update_gui() override;

  void apply(Scene& scene) override;

 private:
  /**
   * @brief モード
   */
  enum class Mode : int {
    DEFAULT,  ///< 通常
    POSITION,  ///< 位置
    NORMAL,  ///< 法線
    AMBIENT,  ///< アンビエント
    DIFFUSE,  ///< ディフューズ
    SPECULAR,  ///< スペキュラ
    SPECULAR_POWER,  ///< スペキュラパワー
  };

  struct Constant {
    Mode mode;
  };

  garie::Program prog_;
  garie::Buffer constant_ub_;
  Mode mode_ = Mode::DEFAULT;
  std::string log_;  // シェーダのエラーログ
};
}  // namespace rtdemo::tech
