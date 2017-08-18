#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace rtdemo {
struct VertexP3 {
  glm::vec3 position;
};

struct VertexP3N3 {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Camera {
  glm::mat4 view_proj;
  glm::mat4 view;
  glm::mat4 proj;
  glm::mat4 view_proj_inv;
  glm::vec3 position_w;
  float _position_w;
};

struct ResourceIndex {
  uint32_t material_index;
};

struct Material {
  glm::vec3 ambient;
  float _ambient;
  glm::vec3 diffuse;
  float _diffuse;
  glm::vec3 specular;
  float specular_power;
};

struct PointLight {
  glm::vec3 position_w;
  float intensity;
  glm::vec3 color;
  float radius;
};
}  // namespace rtdemo
