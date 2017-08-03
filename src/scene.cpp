#include <rendering_techniques/scene.hpp>
#include <vector>
#include <assimp/importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.hpp>
#include <rendering_techniques/scene.hpp>

namespace scene {
struct Vertex {
    float position[3];
}
using Index = uint16_t;

bool Scene::init() {
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
    for (unsigned int i = 0; i < mesh->mNumFaaces; ++i) {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffers[2] = {};
    glGenBuffers(2, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BARRIER, buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(Index), indices.data(), GL_STATIC_DRAW);

    const GLint position_loc = glGetAttribLocation(prog_, "position");
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));

    vao_ = vao;
    vbo_ = buffers[0];
    ibo_ = buffers[1];
    return true;
}

void Scene::draw() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSINED_SHORT, (const GLvoid*)0);
}
}  // namespace scene
