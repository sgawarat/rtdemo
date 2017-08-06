#include <rtdemo/scene/static_scene.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace rtdemo {
namespace scene {
bool StaticScene::init() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("data/scenes/cornellbox/CornellBox-Original.obj", aiProcess_Triangulate);

    std::vector<DrawParam> draw_params;
    draw_params.reserve(scene->mNumMeshes);

    // calc total buffer size
    size_t total_vertex_count = 0;
    size_t total_index_count = 0;
    for (size_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        draw_params.push_back(DrawParam {
            mesh->mMaterialIndex,
            static_cast<GLsizei>(mesh->mNumFaces * 3),
            static_cast<GLsizeiptr>(total_index_count),
            static_cast<GLint>(total_vertex_count)
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
            vertices.push_back(Vertex {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z,
            });
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            indices.push_back(mesh->mFaces[i].mIndices[0]);
            indices.push_back(mesh->mFaces[i].mIndices[1]);
            indices.push_back(mesh->mFaces[i].mIndices[2]);
        }
    }

    // copy all material data
    std::vector<Material> materials;
    materials.reserve(scene->mNumMaterials);
    for (size_t i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* material = scene->mMaterials[i];
        aiColor4D diffuse;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        materials.push_back(Material {
            {diffuse.r, diffuse.g, diffuse.b},
        });
    }

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
        .build();

    garie::Buffer material_ubo;
    material_ubo.create();
    material_ubo.bind(GL_UNIFORM_BUFFER);
    glBufferStorage(GL_UNIFORM_BUFFER, materials.size() * sizeof(Material), materials.data(), 0);

    vao_ = std::move(vao);
    vbo_ = std::move(vbo);
    ibo_ = std::move(ibo);
    material_ubo_ = std::move(material_ubo);
    draw_params_ = std::move(draw_params);
    return true;
}

void StaticScene::apply() const noexcept {
    vao_.bind();
}

void StaticScene::draw() const noexcept {
    for (const auto& draw_param : draw_params_) {
        material_ubo_.bind_range(GL_UNIFORM_BUFFER, 1, draw_param.material_index * sizeof(Material), sizeof(Material));
        // material_ubo_.bind_base(GL_UNIFORM_BUFFER, 1);
        glDrawElementsBaseVertex(GL_TRIANGLES, draw_param.index_count, GL_UNSIGNED_SHORT, (const GLvoid*)(draw_param.index_offset * sizeof(Index)), draw_param.base_vertex);
    }
}
}  // namespace scene
}  // namespace rtdemo
