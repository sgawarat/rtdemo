#include <rtdemo/scene/static_scene.hpp>
#include <vector>
#include <glm/ext.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <imgui.h>
#include <rtdemo/types.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo::scene {
RT_MANAGED_SCENE(StaticScene);

bool StaticScene::restore() {
  // シーンを読み込む
  // const char* scene_path = "assets/scenes/cornellbox/CornellBox-Original.obj";
  const char* scene_path = "assets/scenes/test/untitled.obj";
  Assimp::Importer importer;
  const aiScene* scene =
      importer.ReadFile(scene_path, aiProcess_Triangulate | aiProcess_GenNormals);

  // 描画に必要なデータをコピーする
  size_t total_vertex_count = 0;
  size_t total_index_count = 0;
  std::vector<ResourceIndex> resource_indices;
  std::vector<Command> commands;
  resource_indices.reserve(scene->mNumMeshes);
  commands.reserve(scene->mNumMeshes);
  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    const aiMesh* mesh = scene->mMeshes[i];
    resource_indices.push_back(ResourceIndex{
        mesh->mMaterialIndex,
    });
    commands.push_back(Command{
        static_cast<GLuint>(mesh->mNumFaces * 3), 1,
        static_cast<GLuint>(total_index_count),
        static_cast<GLuint>(total_vertex_count), 0,
    });
    total_index_count += mesh->mNumFaces * 3;
    total_vertex_count += mesh->mNumVertices;
  }

  // メッシュのデータをコピーする
  std::vector<VertexP3N3> vertices;
  std::vector<uint16_t> indices;
  vertices.reserve(total_vertex_count);
  indices.reserve(total_index_count);
  for (size_t mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i) {
    const aiMesh* mesh = scene->mMeshes[mesh_i];
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
      const auto& p = mesh->mVertices[i];
      const auto& n = mesh->mNormals[i];
      vertices.push_back(VertexP3N3{
          {p.x, p.y, p.z}, {n.x, n.y, n.z},
      });
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
      const auto& face = mesh->mFaces[i];
      indices.push_back(static_cast<uint16_t>(face.mIndices[0]));
      indices.push_back(static_cast<uint16_t>(face.mIndices[1]));
      indices.push_back(static_cast<uint16_t>(face.mIndices[2]));
    }
  }

  // マテリアルのデータをコピーする
  std::vector<Material> materials;
  materials.reserve(scene->mNumMaterials);
  for (size_t i = 0; i < scene->mNumMaterials; ++i) {
    const aiMaterial* material = scene->mMaterials[i];
    aiColor4D ambient;
    aiColor4D diffuse;
    aiColor4D specular;
    float shininess = 0.f;
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    material->Get(AI_MATKEY_SHININESS, shininess);
    materials.push_back(Material{
        {ambient.r, ambient.g, ambient.b},
        {/* padding */},
        {diffuse.r, diffuse.g, diffuse.b},
        {/* padding */},
        {specular.r, specular.g, specular.b},
        shininess,
    });
  }

  // ライトのデータをコピーする
  // TODO:シーンから実際のライトデータをコピーする
  std::vector<PointLight> lights;
  lights.reserve(2);
  lights.push_back(PointLight{
    {0.f, 3.f, 2.f},
    7.f,
    {1.f, 1.f, 1.f},
    1.f,
  });
  // lights.push_back(PointLight{
  //   {8.f, 1.f, 8.f},
  //   1.f,
  //   {1.f, 1.f, 1.f},
  //   5.f,
  // });
  // lights.reserve(scene->mNumLights);
  // for (size_t i = 0; i < scene->mNumLights; ++i) {
  //   const aiLight* light = scene->mLights[i];
  //   const auto& p = light->mPosition;
  //   lights.push_back(Layout::Light{
  //     {p.x, p.y, p.z},
  //   });
  // }

  // シャドウキャスタのデータをコピーする
  std::vector<ShadowCaster> shadow_casters;
  shadow_casters.reserve(2);
  shadow_casters.push_back(ShadowCaster{
    glm::perspective(glm::radians(90.f), 1.f, 0.01f, 100.f) * glm::lookAt(glm::vec3(0.f, 5.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
  });

  // GLリソースを生成する
  garie::Buffer vbo;
  vbo.gen();
  vbo.bind(GL_ARRAY_BUFFER);
  glBufferStorage(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexP3N3),
                  vertices.data(), 0);

  garie::Buffer ibo;
  ibo.gen();
  ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,
                  indices.size() * sizeof(uint16_t), indices.data(), 0);

  garie::VertexArray vao = garie::VertexArrayBuilder()
      .index_buffer(ibo)
      .vertex_buffer(vbo)
      .attribute(0, 3, GL_FLOAT, GL_FALSE,
                 sizeof(VertexP3N3), offsetof(VertexP3N3, position), 0)
      .attribute(1, 3, GL_FLOAT, GL_FALSE,
                 sizeof(VertexP3N3), offsetof(VertexP3N3, normal), 0)
      .build();

  garie::Buffer camera_ubo;
  camera_ubo.gen();
  camera_ubo.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(
      GL_UNIFORM_BUFFER, sizeof(Camera), nullptr, GL_MAP_WRITE_BIT);

  garie::Buffer resource_index_ssbo;
  resource_index_ssbo.gen();
  resource_index_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  resource_indices.size() * sizeof(ResourceIndex),
                  resource_indices.data(), 0);

  garie::Buffer material_ssbo;
  material_ssbo.gen();
  material_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  materials.size() * sizeof(Material), materials.data(),
                  0);

  garie::Buffer light_ssbo;
  light_ssbo.gen();
  light_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  lights.size() * sizeof(PointLight), lights.data(), 0);

  garie::Buffer shadow_ssbo;
  shadow_ssbo.gen();
  shadow_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  shadow_casters.size() * sizeof(ShadowCaster), shadow_casters.data(), 0);

  garie::Buffer dio;
  dio.gen();
  dio.bind(GL_DRAW_INDIRECT_BUFFER);
  glBufferStorage(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command),
                  commands.data(), 0);

  // 後始末
  camera_center_ = 0.f;
  camera_distance_ = 10.f;
  camera_yaw_ = 0.f;
  camera_pitch_ = glm::radians(-45.f);
  draw_mode_ = DrawMode::DRAW;

  vao_ = std::move(vao);
  vbo_ = std::move(vbo);
  ibo_ = std::move(ibo);
  camera_ubo_ = std::move(camera_ubo);
  resource_index_ssbo_ = std::move(resource_index_ssbo);
  material_ssbo_ = std::move(material_ssbo);
  light_ssbo_ = std::move(light_ssbo);
  shadow_ssbo_ = std::move(shadow_ssbo);
  light_count_ = lights.size();
  dio_ = std::move(dio);
  commands_ = std::move(commands);
  return true;
}

bool StaticScene::invalidate() {
  vao_ = garie::VertexArray();
  vbo_ = garie::Buffer();
  ibo_ = garie::Buffer();
  camera_ubo_ = garie::Buffer();
  resource_index_ssbo_ = garie::Buffer();
  material_ssbo_ = garie::Buffer();
  light_ssbo_ = garie::Buffer();
  shadow_ssbo_ = garie::Buffer();
  light_count_ = 0;
  dio_ = garie::Buffer();
  commands_.clear();
  return true;
}

void StaticScene::update() {
  // 射影行列を計算する
  const glm::mat4 proj =
      glm::perspective(glm::radians(45.f), 1280.f / 720.f, 0.1f, lens_depth_);

  // ビュー行列を計算する
  const glm::mat3 rot = glm::yawPitchRoll(camera_yaw_, camera_pitch_, 0.f);
  const glm::vec3 eye = rot * glm::vec3(0.f, 0.f, camera_distance_);
  const glm::vec3 front = rot * glm::vec3(0.f, 0.f, -1.f);
  const glm::vec3 up = rot * glm::vec3(0.f, 1.f, 0.f);
  const glm::mat4 view = glm::lookAt(eye, glm::vec3(0.f, camera_center_, 0.f), up);
  const glm::mat4 view_proj = proj * view;

  // カメラ情報を更新する
  camera_ubo_.bind(GL_UNIFORM_BUFFER);
  Camera* mapped_camera_ubo =
  reinterpret_cast<Camera*>(glMapBufferRange(
      GL_UNIFORM_BUFFER, 0, sizeof(Camera),
      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  mapped_camera_ubo->view_proj = view_proj;
  mapped_camera_ubo->view = view;
  mapped_camera_ubo->proj = proj;
  mapped_camera_ubo->view_proj_inv = glm::inverse(view_proj);
  mapped_camera_ubo->view_inv = glm::inverse(view);
  mapped_camera_ubo->proj_inv = glm::inverse(proj);
  mapped_camera_ubo->position_w = eye;
  glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void StaticScene::update_gui() {
  ImGui::Begin("StaticScene");
  ImGui::DragFloat("center", &camera_center_, 0.01f, -20.f, 20.f);
  ImGui::DragFloat("distance", &camera_distance_, 0.01f, 0.01f, 20.f);
  ImGui::SliderAngle("yaw", &camera_yaw_, -180.f, 180.f);
  ImGui::SliderAngle("pitch", &camera_pitch_, -90.f, 90.f);
  ImGui::DragFloat("depth", &lens_depth_, 0.01f, 0.01f, 30.f);
  ImGui::Combo("draw mode", reinterpret_cast<int*>(&draw_mode_),
               "DRAW\0DRAW_INDIRECT\0\0");
  ImGui::End();
}

void StaticScene::apply(ApplyType type) {
  switch (type) {
    case ApplyType::SHADE: {
      camera_ubo_.bind_base(GL_UNIFORM_BUFFER, 0);
      
      resource_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 0);
      material_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 1);
      light_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 2);
      shadow_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 3);
      break;
    }
    case ApplyType::NO_SHADE: {
      camera_ubo_.bind_base(GL_UNIFORM_BUFFER, 0);
      break;
    }
    case ApplyType::LIGHT: {
      camera_ubo_.bind_base(GL_UNIFORM_BUFFER, 0);
      
      light_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 0);
      break;
    }
    case ApplyType::SHADOW: {
      shadow_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 0);
      break;
    }
  }
}

void StaticScene::draw(DrawType type) {
  switch (type) {
    case DrawType::OPAQUE: {
      vao_.bind();
      switch (draw_mode_) {
        case DrawMode::DRAW: {
          for (size_t i = 0; i < commands_.size(); ++i) {
            const auto& command = commands_[i];
            glUniform1ui(10, static_cast<GLuint>(i));
            glDrawElementsInstancedBaseVertexBaseInstance(
                GL_TRIANGLES, command.index_count, GL_UNSIGNED_SHORT,
                (const GLvoid*)(command.index_first * sizeof(uint16_t)),
                command.instance_count, command.base_vertex, command.base_instance);
          }
          break;
        }
        case DrawMode::DRAW_INDIRECT: {
          dio_.bind(GL_DRAW_INDIRECT_BUFFER);
          for (size_t i = 0; i < commands_.size(); ++i) {
            const auto& command = commands_[i];
            glUniform1ui(10, static_cast<GLuint>(i));
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                                  (const void*)(i * sizeof(Command)));
          }
          glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
          break;
        }
        break;
      }
      break;
    }
    case DrawType::TRANSPARENT: {
      break;
    }
    case DrawType::LIGHT_VOLUME: {
      util::screen_quad_vao().bind();
      for (size_t i = 0; i < light_count_; ++i) {
        glUniform1ui(10, static_cast<GLuint>(i));
        util::draw_light_quad();
      }
      break;
    }
  }
}
}  // namespace rtdemo::scene
