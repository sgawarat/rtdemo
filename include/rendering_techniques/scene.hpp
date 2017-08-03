#include <GLAD/glad.h>

namespace scene {
class Scene {
public:
    Scene() noexcept {}

    bool load();

    void draw();

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ibo_ = 0;
    GLsizei count_ = 0;
};
} // namespace scene
