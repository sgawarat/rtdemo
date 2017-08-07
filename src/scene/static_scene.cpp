#include <rtdemo/scene/static_scene.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace rtdemo {
namespace scene {
bool StaticScene::init() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("assets/scenes/cornellbox/CornellBox-Original.obj", aiProcess_Triangulate | aiProcess_GenNormals);

    std::vector<ResourceIndex> resource_indices;
    std::vector<Command> commands;
    resource_indices.reserve(scene->mNumMeshes);
    commands.reserve(scene->mNumMeshes);

    // calc total buffer size
    size_t total_vertex_count = 0;
    size_t total_index_count = 0;
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        resource_indices.push_back(ResourceIndex {
            mesh->mMaterialIndex,
        });
        commands.push_back(Command {
            static_cast<GLuint>(mesh->mNumFaces * 3),
            1,
            static_cast<GLuint>(total_index_count),
            static_cast<GLuint>(total_vertex_count),
            0,
        });
        total_index_count += mesh->mNumFaces * 3;
        total_vertex_count += mesh->mNumVertices;
    }

    // copy all mesh data
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    vertices.reserve(total_vertex_count);
    indices.reserve(total_index_count);
    for (size_t mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i) {
        const aiMesh* mesh = scene->mMeshes[mesh_i];
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            const auto& p = mesh->mVertices[i];
            const auto& n = mesh->mNormals[i];
            vertices.push_back(Vertex {
                {p.x, p.y, p.z},
                {n.x, n.y, n.z},
            });
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            const auto& face = mesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }

    // copy all material data
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
        materials.push_back(Material {
            {ambient.r, ambient.g, ambient.b}, {},
            {diffuse.r, diffuse.g, diffuse.b}, {},
            {specular.r, specular.g, specular.b}, shininess,
        });
    }

    // copy all light data
    std::vector<Light> lights;
    lights.reserve(1);
    lights.push_back(Light {
        {0.f, 2.f, 0.f},
    });

    // create OpenGL resources
    garie::Buffer vbo;
    vbo.create();
    vbo.bind(GL_ARRAY_BUFFER);
    glBufferStorage(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), 0);

    garie::Buffer ibo;
    ibo.create();
    ibo.bind(GL_ELEMENT_ARRAY_BUFFER);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), 0);

    garie::VertexArray vao;
    vao.create();
    garie::VertexArrayBuilder vao_builder(vao);
    vao_builder.index_buffer(ibo)
        .vertex_buffer(vbo)
        .attribute(IN_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position), 0)
        .attribute(IN_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal), 0)
        .build();

    garie::Buffer resource_indices_ssbo;
    resource_indices_ssbo.create();
    resource_indices_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, resource_indices.size() * sizeof(ResourceIndex), resource_indices.data(), 0);

    garie::Buffer materials_ssbo;
    materials_ssbo.create();
    materials_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, materials.size() * sizeof(Material), materials.data(), 0);

    garie::Buffer lights_ssbo;
    lights_ssbo.create();
    lights_ssbo.bind(GL_SHADER_STORAGE_BUFFER);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(Light), lights.data(), 0);

    garie::Buffer dio;
    dio.create();
    dio.bind(GL_DRAW_INDIRECT_BUFFER);
    glBufferStorage(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), commands.data(), 0);

    vao_ = std::move(vao);
    vbo_ = std::move(vbo);
    ibo_ = std::move(ibo);
    resource_indices_ssbo_ = std::move(resource_indices_ssbo);
    materials_ssbo_ = std::move(materials_ssbo);
    lights_ssbo_ = std::move(lights_ssbo);
    dio_ = std::move(dio);
    commands_ = std::move(commands);
    return true;
}

void StaticScene::apply() const noexcept {
    vao_.bind();
    resource_indices_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 0);
    materials_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 1);
    lights_ssbo_.bind_base(GL_SHADER_STORAGE_BUFFER, 2);

    glEnable(GL_CULL_FACE);
}

void StaticScene::draw() const noexcept {
    switch (draw_mode_) {
        case DrawMode::DRAW: {
            for (size_t i = 0; i < commands_.size(); ++i) {
                const auto& command = commands_[i];
                glUniform1ui(10, static_cast<GLuint>(i));
                glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, command.index_count, GL_UNSIGNED_SHORT, (const GLvoid*)(command.index_first * sizeof(Index)), command.instance_count, command.base_vertex, command.base_instance);
            }
            break;
        }
        case DrawMode::DRAW_INDIRECT: {
            dio_.bind(GL_DRAW_INDIRECT_BUFFER);
            for (size_t i = 0; i < commands_.size(); ++i) {
                const auto& command = commands_[i];
                glUniform1ui(10, static_cast<GLuint>(i));
                glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)(i * sizeof(Command)));
            }
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
            break;
        }
    }
    // glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, commands_.data(), commands_.size(), sizeof(Command));
}
}  // namespace scene
}  // namespace rtdemo
