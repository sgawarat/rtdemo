#include <rtdemo/application.hpp>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <imgui.h>
#include <rtdemo/managed.hpp>

namespace rtdemo {
namespace {
class ManagedTechniqueList {
 public:
  void push(const char* name, tech::Technique* technique) {
    names_.push_back(name);
    techniques_.push_back(technique);
  }

  const std::vector<const char*>& names() const noexcept {
    return names_;
  }

  const std::vector<tech::Technique*>& techniques() const noexcept {
    return techniques_;
  }

 private:
  std::vector<const char*> names_;
  std::vector<tech::Technique*> techniques_;
};

class ManagedSceneList {
 public:
  void push(const char* name, scene::Scene* scene) {
    names_.push_back(name);
    scenes_.push_back(scene);
  }

  const std::vector<const char*>& names() const noexcept {
    return names_;
  }

  const std::vector<scene::Scene*>& scenes() const noexcept {
    return scenes_;
  }

 private:
  std::vector<const char*> names_;
  std::vector<scene::Scene*> scenes_;
};

std::vector<const char*> layout_names;
std::vector<uint64_t> layout_ids;
std::unordered_map<uint64_t, ManagedTechniqueList> technique_list_map;
std::unordered_map<uint64_t, ManagedSceneList> scene_list_map;

int current_layout_index = -1;
uint64_t current_layout_id = 0;
ManagedTechniqueList* current_technique_list = nullptr;
ManagedSceneList* current_scene_list = nullptr;
int current_technique_index = -1;
int current_scene_index = -1;
tech::Technique* current_technique = nullptr;
scene::Scene* current_scene = nullptr;
}  // namespace

void register_managed_layout(const char* name, uint64_t id) {
  layout_names.push_back(name);
  layout_ids.push_back(id);
}

void register_managed_technique(const uint64_t* layout_ids,
                                size_t layout_id_count, const char* name,
                                tech::Technique* technique) {
  for (size_t i = 0; i < layout_id_count; ++i) {
    auto& technique_list = technique_list_map[layout_ids[i]];
    technique_list.push(name, technique);
  }
}

void register_managed_scene(const uint64_t* layout_ids, size_t layout_id_count,
                            const char* name, scene::Scene* scene) {
  for (size_t i = 0; i < layout_id_count; ++i) {
    auto& scene_list = scene_list_map[layout_ids[i]];
    scene_list.push(name, scene);
  }
}

bool Application::init() {
  current_layout_id = layout_ids[0];
  if (!current_layout_id) return false;
  current_layout_index = 0;

  current_technique_list = &technique_list_map[current_layout_id];
  current_scene_list = &scene_list_map[current_layout_id];

  current_technique = current_technique_list->techniques()[0];
  if (!current_technique) return false;
  current_technique_index = 0;
  current_technique->restore();

  current_scene = current_scene_list->scenes()[0];
  if (!current_scene) return false;
  current_scene_index = 0;
  current_scene->restore();

  return true;
}

void Application::update() {
  if (ImGui::Combo("current layout", &current_layout_index, layout_names.data(),
                   layout_names.size())) {
    current_layout_id = layout_ids[current_layout_index];
    current_technique_list = &technique_list_map[current_layout_id];
    current_scene_list = &scene_list_map[current_layout_id];
    current_technique_index = -1;
    current_scene_index = -1;
    current_technique = nullptr;
    current_scene = nullptr;
  }

  if (current_technique_list) {
    if (ImGui::Combo("current technique", &current_technique_index,
                     current_technique_list->names().data(),
                     current_technique_list->names().size())) {
      current_technique =
          current_technique_list->techniques()[current_technique_index];
      current_technique->restore();
    }
  }

  if (current_scene_list) {
    if (ImGui::Combo("current scene", &current_scene_index,
                     current_scene_list->names().data(),
                     current_scene_list->names().size())) {
      current_scene = current_scene_list->scenes()[current_scene_index];
      current_scene->restore();
    }
  }

  if (current_technique) {
    current_technique->update_gui();
  }
  if (current_scene) {
    current_scene->update_gui();
  }
  if (current_technique && current_scene) {
    current_technique->update();
    current_scene->update();
    current_technique->apply(current_scene);
  }
}
}  // namespace rtdemo
