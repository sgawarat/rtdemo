#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace rtdemo {
/**
 * @brief 3次元の位置を持つ頂点
 * 
 */
struct VertexP3 {
  glm::vec3 position;  ///< 位置
};

/**
 * @brief 3次元の位置と3次元の法線を持つ頂点
 * 
 */
struct VertexP3N3 {
  glm::vec3 position;  ///< 位置
  glm::vec3 normal;  ///< 法線
};

/**
 * @brief カメラ
 * 
 */
struct Camera {
  glm::mat4 view_proj;  ///< ビュー行列×射影行列
  glm::mat4 view;  ///< ビュー行列
  glm::mat4 proj;  ///< 射影行列
  glm::mat4 view_proj_inv;  ///< view_projの逆行列
  glm::mat4 view_inv;  ///< viewの逆行列
  glm::mat4 proj_inv;  ///< projの逆行列
  glm::vec4 range;  ///< x:幅、y:高さ、z:ニア面、w:ファー面
  glm::vec3 position_w;  ///< ワールド座標における位置
  float _position_w;  ///< パッディング
};

/**
 * @brief 
 * 
 */
struct ResourceIndex {
  uint32_t material_index;
};

/**
 * @brief マテリアル
 * 
 */
struct Material {
  glm::vec3 ambient;  ///< アンビエント
  float _ambient;  ///< パッディング
  glm::vec3 diffuse;  ///< ディフューズ
  float _diffuse;  ///< パッディング
  glm::vec3 specular;  ///< スペキュラ
  float specular_power;  ///< スペキュラパワー
};

/**
 * @brief 点光源
 * 
 */
struct PointLight {
  glm::vec3 position_w;  ///< ワールド座標における位置
  float radius;  ///< 半径
  glm::vec3 color;  ///< 色
  float intensity;  ///< 強度
};

/**
 * @brief シャドウキャスタ
 * 
 */
struct ShadowCaster {
  glm::mat4 view_proj;  ///< ビュー行列×射影行列
};
}  // namespace rtdemo
