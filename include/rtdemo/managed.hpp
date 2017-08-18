#pragma once

#include <cstdint>
#include "scene/scene.hpp"
#include "tech/technique.hpp"

#define RT_MANAGED_SCENE_INSTANCE(ns, T)                        \
namespace {                                                   \
::rtdemo::ManagedScene<ns::T> MANAGED_SCENE_INSTANCE_##T{#T}; \
}

#define RT_MANAGED_TECHNIQUE_INSTANCE(ns, T)                            \
  namespace {                                                           \
  ::rtdemo::ManagedTechnique<ns::T> MANAGED_TECHNIQUE_INSTANCE_##T{#T}; \
  }

namespace rtdemo {
  void register_managed_scene(const char*, scene::Scene*);
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
