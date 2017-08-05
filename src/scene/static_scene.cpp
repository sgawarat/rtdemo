#include <rtdemo/scene/static_scene.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace rtdemo {
namespace scene {
bool StaticScene::init() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("data/scenes/cornellbox/CornellBox-Sphere.obj", 0);
    const aiMesh* mesh = scene->mMeshes[0];

    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vertices.push_back(Vertex {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
        });
    }

    std::vector<Index> indices;
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

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

    vao_ = std::move(vao);
    vbo_ = std::move(vbo);
    ibo_ = std::move(ibo);
    count_ = mesh->mNumFaces * 3;
    return true;
}

void StaticScene::apply() const noexcept {
    vao_.bind();
}

void StaticScene::draw() const noexcept {
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_SHORT, (const GLvoid*)0);
}
}  // namespace scene
}  // namespace rtdemo
