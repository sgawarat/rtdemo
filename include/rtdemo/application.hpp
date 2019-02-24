#pragma once

#include <memory>
#include <map>
#include <string>
#include <cstdint>
#ifdef WIN32
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>

namespace rtdemo {
class Scene;
class Technique;

class Application final {
 public:
  /**
   * @brief Applicationのインスタンスを取得する
   * 
   * @return Application& 
   */
  static Application& get() noexcept;

  /**
   * @brief 初期化する
   * 
   * @param screen_width バックバッファの幅
   * @param screen_height バックバッファの高さ
   * @return true 成功した
   * @return false 失敗した
   */
  bool init(size_t screen_width, size_t screen_height);

  /**
   * @brief 破棄する
   */
  void terminate();

  /**
   * @brief 更新する
   * 
   * @return true 成功した
   * @return false 失敗した
   */
  bool update();

  /**
   * @brief 一覧にシーンを登録する
   * 
   * @param name シーン名
   * @param scene シーンの実体
   * @return true 成功した
   * @return false 失敗した
   */
  bool insert_scene(std::string name, std::shared_ptr<Scene> scene);

  /**
   * @brief 一覧にテクニックを登録する
   * 
   * @param name テクニック名
   * @param scene テクニックの実体
   * @return true 成功した
   * @return false 失敗した
   */
  bool insert_technique(std::string name, std::shared_ptr<Technique> technique);

  uint32_t screen_width() const noexcept {
    return screen_width_;
  }

  uint32_t screen_height() const noexcept {
    return screen_height_;
  }

 private:
  using SceneMap = std::map<std::string, std::shared_ptr<Scene>>;
  using TechniqueMap = std::map<std::string, std::shared_ptr<Technique>>;


  GLFWwindow* window_ = nullptr;  ///< ウィンドウハンドル
  uint32_t screen_width_ = 0;  ///< バックバッファの幅
  uint32_t screen_height_ = 0;  ///< バックバッファの高さ

  // 実体
  SceneMap scene_map_;  ///< シーンを名前で検索するためのマップ
  TechniqueMap technique_map_;  ///< テクニックを名前で検索するためのマップ

  // 更新や表示を行う対象
  SceneMap::const_iterator current_scene_;  ///< 現在のシーン
  TechniqueMap::const_iterator current_technique_;  ///< 現在のテクニック
};
}  // namespace rtdemo
