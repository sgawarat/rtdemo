#include <rtdemo/scene/static_scene.hpp>
#include <vector>
#include <glm/ext.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <imgui.h>
#include <rtdemo/managed.hpp>

namespace rtdemo {
RT_MANAGED_SCENE_INSTANCE(scene, StaticScene);

namespace scene {
bool StaticScene::restore() {
  // const char* scene_path = "assets/scenes/cornellbox/CornellBox-Original.obj";
  const char* scene_path = "assets/scenes/test/untitled.obj";
  Assimp::Importer importer;
  const aiScene* scene =
      importer.ReadFile(scene_path, aiProcess_Triangulate | aiProcess_GenNormals);

  // copy all draw data
  size_t total_vertex_count = 0;
  size_t total_index_count = 0;
  std::vector<Layout::ResourceIndex> resource_indices;
  std::vector<Command> commands;
  resource_indices.reserve(scene->mNumMeshes);
  commands.reserve(scene->mNumMeshes);
  for (size_t i = 0; i < scene->mNumMeshes; ++i) {
    const aiMesh* mesh = scene->mMeshes[i];
    resource_indices.push_back(Layout::ResourceIndex{
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

  // copy all mesh data
  std::vector<Layout::Vertex> vertices;
  std::vector<Layout::Index> indices;
  vertices.reserve(total_vertex_count);
  indices.reserve(total_index_count);
  for (size_t mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i) {
    const aiMesh* mesh = scene->mMeshes[mesh_i];
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
      const auto& p = mesh->mVertices[i];
      const auto& n = mesh->mNormals[i];
      vertices.push_back(Layout::Vertex{
          {p.x, p.y, p.z}, {n.x, n.y, n.z},
      });
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
      const auto& face = mesh->mFaces[i];
      indices.push_back(static_cast<Layout::Index>(face.mIndices[0]));
      indices.push_back(static_cast<Layout::Index>(face.mIndices[1]));
      indices.push_back(static_cast<Layout::Index>(face.mIndices[2]));
    }
  }

  // copy all material data
  std::vector<Layout::Material> materials;
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
    materials.push_back(Layout::Material{
        {ambient.r, ambient.g, ambient.b},
        {},
        {diffuse.r, diffuse.g, diffuse.b},
        {},
        {specular.r, specular.g, specular.b},
        shininess,
    });
  }

  // copy all light data
   std::vector<Layout::Light> lights;
  lights.reserve(2);
  lights.push_back(Layout::Light{
      {0.f, 5.f, 0.f},
  });
  lights.push_back(Layout::Light{
    {8.f, 1.f, 8.f},
});
// lights.reserve(scene->mNumLights);
  // for (size_t i = 0; i < scene->mNumLights; ++i) {
  //   const aiLight* light = scene->mLights[i];
  //   const auto& p = light->mPosition;
  //   lights.push_back(Layout::Light{
  //     {p.x, p.y, p.z},
  //   });
  // }

  // create OpenGL resources
  garie::Buffer vbo;
  vbo.gen();
  vbo.bind(GL_ARRAY_BUFFER);
  glBufferStorage(GL_ARRAY_BUFFER, vertices.size() * sizeof(Layout::Vertex),
                  vertices.data(), 0);

  garie::Buffer ibo;
  ibo.gen();
  ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,
                  indices.size() * sizeof(Layout::Index), indices.data(), 0);

  garie::VertexArray vao = garie::VertexArrayBuilder()
      .index_buffer(ibo)
      .vertex_buffer(vbo)
      .attribute(Layout::AttributeLocation::POSITION, 3, GL_FLOAT, GL_FALSE,
                 sizeof(Layout::Vertex), offsetof(Layout::Vertex, position), 0)
      .attribute(Layout::AttributeLocation::NORMAL, 3, GL_FLOAT, GL_FALSE,
                 sizeof(Layout::Vertex), offsetof(Layout::Vertex, normal), 0)
      .build();

  garie::Buffer camera_ubo;
  camera_ubo.gen();
  camera_ubo.bind(GL_UNIFORM_BUFFER);
  glBufferStorage(
      GL_UNIFORM_BUFFER, sizeof(Layout::Camera), nullptr, GL_MAP_WRITE_BIT);

  garie::Buffer resource_index_ssbo;
  resource_index_ssbo.gen();
  resource_index_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  resource_indices.size() * sizeof(Layout::ResourceIndex),
                  resource_indices.data(), 0);

  garie::Buffer material_ssbo;
  material_ssbo.gen();
  material_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  materials.size() * sizeof(Layout::Material), materials.data(),
                  0);

  garie::Buffer light_ssbo;
  light_ssbo.gen();
  light_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER,
                  lights.size() * sizeof(Layout::Light), lights.data(), 0);

  garie::Buffer dio;
  dio.gen();
  dio.bind(GL_DRAW_INDIRECT_BUFFER);
  glBufferStorage(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command),
                  commands.data(), 0);

  // finalize
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
  dio_ = garie::Buffer();
  commands_.clear();
  return true;
}

void StaticScene::update() {
  const glm::mat4 proj =
      glm::perspective(glm::radians(45.f), 1280.f / 720.f, 0.1f, lens_depth_);

  const glm::mat3 rot = glm::yawPitchRoll(camera_yaw_, camera_pitch_, 0.f);
  const glm::vec3 eye = rot * glm::vec3(0.f, 0.f, camera_distance_);
  const glm::vec3 front = rot * glm::vec3(0.f, 0.f, -1.f);
  const glm::vec3 up = rot * glm::vec3(0.f, 1.f, 0.f);
  const glm::mat4 view = glm::lookAt(eye, glm::vec3(0.f, camera_center_, 0.f), up);
  const glm::mat4 view_proj = proj * view;

  // update camera
  camera_ubo_.bind(GL_UNIFORM_BUFFER);
  Layout::Camera* mapped_camera_ubo =
  reinterpret_cast<Layout::Camera*>(glMapBufferRange(
      GL_UNIFORM_BUFFER, 0, sizeof(Layout::Camera),
      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  mapped_camera_ubo->view_proj = view_proj;
  mapped_camera_ubo->view_proj_inv = glm::inverse(view_proj);
  mapped_camera_ubo->position_w = eye;
  glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void StaticScene::update_gui() {
  ImGui::DragFloat("center", &camera_center_, 0.01f, -20.f, 20.f);
  ImGui::DragFloat("distance", &camera_distance_, 0.01f, 0.01f, 20.f);
  ImGui::SliderAngle("yaw", &camera_yaw_, -180.f, 180.f);
  ImGui::SliderAngle("pitch", &camera_pitch_, -90.f, 90.f);
  ImGui::DragFloat("depth", &lens_depth_, 0.01f, 0.01f, 30.f);
  ImGui::Combo("draw mode", reinterpret_cast<int*>(&draw_mode_),
               "DRAW\0DRAW_INDIRECT\0\0");
}

void StaticScene::apply(size_t draw_index) {
  // bind uniform buffers
  camera_ubo_.bind_base(GL_UNIFORM_BUFFER, Layout::UniformBinding::CAMERA);
  
  // bind storage buffers
  resource_index_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER,
                                 Layout::StorageBinding::RESOURCE_INDEX);
  material_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER,
                           Layout::StorageBinding::MATERIAL);
  light_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER,
                        Layout::StorageBinding::LIGHT);
}

void StaticScene::draw(size_t draw_index) {
  // bind vertex array
  vao_.bind();

  // draw
  switch (draw_mode_) {
    case DrawMode::DRAW: {
      for (size_t i = 0; i < commands_.size(); ++i) {
        const auto& command = commands_[i];
        glUniform1ui(Layout::ConstantLocation::DRAW_ID, static_cast<GLuint>(i));
        glDrawElementsInstancedBaseVertexBaseInstance(
            GL_TRIANGLES, command.index_count, GL_UNSIGNED_SHORT,
            (const GLvoid*)(command.index_first * sizeof(Layout::Index)),
            command.instance_count, command.base_vertex, command.base_instance);
      }
      break;
    }
    case DrawMode::DRAW_INDIRECT: {
      dio_.bind(GL_DRAW_INDIRECT_BUFFER);
      for (size_t i = 0; i < commands_.size(); ++i) {
        const auto& command = commands_[i];
        glUniform1ui(Layout::ConstantLocation::DRAW_ID, static_cast<GLuint>(i));
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                               (const void*)(i * sizeof(Command)));
      }
      glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
      break;
    }
  }
}
}  // namespace scene
}  // namespace rtdemo
