#pragma once

#include <cstdint>
#include <rtdemo/tech/technique.hpp>
#include <rtdemo/scene/scene.hpp>

#define RT_MANAGED_LAYOUT_INSTANCE(ns, T)\
namespace {\
::rtdemo::ManagedLayout<ns::T> MANAGED_LAYOUT_INSTANCE_##T {#T};\
}

#define RT_MANAGED_TECHNIQUE_INSTANCE(ns, T)\
namespace {\
::rtdemo::ManagedTechnique<ns::T> MANAGED_TECHNIQUE_INSTANCE_##T {#T};\
}

#define RT_MANAGED_SCENE_INSTANCE(ns, T)\
namespace {\
::rtdemo::ManagedScene<ns::T> MANAGED_SCENE_INSTANCE_##T {#T};\
}

namespace rtdemo {
void register_managed_layout(const char* name, uint64_t id);
void register_managed_technique(const uint64_t*, size_t, const char*, tech::Technique*);
void register_managed_scene(const uint64_t*, size_t, const char*, scene::Scene*);

template <typename T>
class ManagedLayout final {
public:
    ManagedLayout(const char* name) {
        register_managed_layout(name, T::ID);
    }

private:
};

template <typename T>
class ManagedTechnique final {
public:
    ManagedTechnique(const char* name) {
        using Layout = typename T::Layout;
        register_managed_technique(&Layout::ID, 1, name, &technique_);
    }

private:
    T technique_;
};

template <typename T>
class ManagedScene final {
public:
    ManagedScene(const char* name) {
        using Layout = typename T::Layout;
        register_managed_scene(&Layout::ID, 1, name, &scene_);
    }

private:
    T scene_;
};
}  // namespace rtdemo
