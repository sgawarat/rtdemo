#pragma once

#include "application.hpp"

namespace rtdemo {
/**
 * @brief バインドするリソースの種類
 */
enum class ApplyType {
  /**
   * @brief シェーディングあり
   * 
   * UBO = {
   *   0: カメラ
   * }
   * SSBO = {
   *   0: リソース番号
   *   1: マテリアル
   *   2: ライト
   *   3: シャドウ
   * }
   */
  SHADE,

  /**
   * @brief シェーディングなし
   * 
   * UBO = {
   *   0: カメラ
   * }
   */
  NO_SHADE,

  /**
   * @brief ライト情報のみ
   * 
   * UBO = {
   *   0: カメラ
   * }
   * SSBO = {
   *   0: ライト
   * }
   */
  LIGHT,

  /**
   * @brief シャドウ情報のみ
   * 
   * SSBO = {
   *   0: シャドウ
   * }
   */
  SHADOW,

  /**
   * @brief ライト情報のみ
   * 
   * UBO = {
   *   0: カメラ
   * }
   * SSBO = {
   *   0: ライト
   *   1: シャドウ
   * }
   */
  LIGHT_SHADOW,
};

/**
 * @brief 描画の種類
 */
enum class DrawType {
  /**
   * @brief 不透明オブジェクトを描画する
   */
  OPAQUE,

  /**
   * @brief 半透明オブジェクトを描画する
   */
  TRANSPARENT,

  /**
   * @brief ライトボリュームを描画する
   */
  LIGHT_VOLUME,
};

/**
 * @brief シーンのインターフェイス
 * 
 * Sceneはシーンの状態を保持し、ジオメトリを描画する方法を定義する。
 */
class Scene {
 public:
  virtual ~Scene() noexcept {}

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
   * @brief シーンの状態を更新する
   */
  virtual void update() = 0;

  /**
   * @brief GUIを更新する
   */
  virtual void update_gui() = 0;

  /**
   * @brief リソースをバインドする
   * 
   * @param type バインドするリソースの種類
   */
  virtual void apply(ApplyType type) = 0;

  /**
   * @brief 描画する
   * 
   * @param type 描画の種類
   */
  virtual void draw(DrawType type) = 0;
};

/**
 * @brief staticなシーンを定義するマクロ
 * 
 * ソースファイルでrtdemo::scene下に記述すると、プログラム起動時にT型のシーンを登録してくれる。
 */
#define RT_MANAGED_SCENE(T) \
  namespace { \
    static struct ManagedScene_##T { \
      ManagedScene_##T() { \
        ::rtdemo::Application::get().insert_scene(#T, std::make_shared<T>()); \
      } \
    } managed_scene_##T##_; \
  }
}  // namespace rtdemo
