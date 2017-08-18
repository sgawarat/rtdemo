#include <rtdemo/application.hpp>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <imgui.h>
#include <rtdemo/managed.hpp>

namespace rtdemo {
namespace {
std::vector<const char*> scene_names;
std::vector<scene::Scene*> scenes;
std::vector<const char*> technique_names;
std::vector<tech::Technique*> techniques;

int current_scene_index = -1;
scene::Scene* current_scene = nullptr;
int current_technique_index = -1;
tech::Technique* current_technique = nullptr;
}  // namespace

void register_managed_scene(const char* name, scene::Scene* scene) {
  scene_names.push_back(name);
  scenes.push_back(scene);
}

void register_managed_technique(const char* name, tech::Technique* technique) {
  technique_names.push_back(name);
  techniques.push_back(technique);
}

bool Application::init() {
  if (scenes.empty()) {
    current_scene_index = -1;
    current_scene = nullptr;
  } else {
    current_scene_index = 0;
    current_scene = scenes[0];
    if (current_scene) current_scene->restore();
  }

  if (techniques.empty()) {
    current_technique_index = -1;
    current_technique = nullptr;
  } else {
    current_technique_index = 0;
    current_technique = techniques[0];
    if (current_technique) current_technique->restore();
  }

  return true;
}

void Application::update() {
  if (ImGui::Combo("current scene", &current_scene_index, scene_names.data(),
                   scene_names.size())) {
    current_scene = scenes[current_scene_index];
    if (current_scene) current_scene->restore();
  }

  if (ImGui::Combo("current technique", &current_technique_index,
                   technique_names.data(), technique_names.size())) {
    current_technique = techniques[current_technique_index];
    if (current_technique) current_technique->restore();
  }

  if (current_scene) {
    current_scene->update_gui();
  }
  if (current_technique) {
    current_technique->update_gui();
  }
  if (current_scene && current_technique) {
    current_scene->update();
    current_technique->update();
    current_technique->apply(current_scene);
  }
}
}  // namespace rtdemo
