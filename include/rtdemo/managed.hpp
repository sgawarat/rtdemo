#pragma once

#include <cstdint>
#include "scene/scene.hpp"
#include "tech/technique.hpp"

/**
 * @brief ユニークなManagedSceneを定義するマクロ
 * 
 */
#define RT_MANAGED_SCENE_INSTANCE(ns, T)                          \
  namespace {                                                     \
    ::rtdemo::ManagedScene<ns::T> MANAGED_SCENE_INSTANCE_##T{#T}; \
  }

/**
 * @brief ユニークなManagedTechniqueを定義するマクロ
 * 
 */
#define RT_MANAGED_TECHNIQUE_INSTANCE(ns, T)                              \
  namespace {                                                             \
    ::rtdemo::ManagedTechnique<ns::T> MANAGED_TECHNIQUE_INSTANCE_##T{#T}; \
  }

namespace rtdemo {
/**
 * @brief シーンをリストに登録する
 * 
 */
void register_managed_scene(const char*, scene::Scene*);

/**
 * @brief テクニックをリストに登録する
 * 
 */
void register_managed_technique(const char*, tech::Technique*);

template <typename T>
class ManagedScene final {
 public:
  ManagedScene(const char* name) {
    register_managed_scene(name, &scene_);
  }

 private:
  T scene_;
};

template <typename T>
class ManagedTechnique final {
 public:
  ManagedTechnique(const char* name) {
    register_managed_technique(name, &technique_);
  }

 private:
  T technique_;
};
}  // namespace rtdemo
