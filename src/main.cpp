#include <iostream>
#include <cstdlib>
#include <rendering_techniques/stable.hpp>
#include <rendering_techniques/gui.hpp>

namespace {
void glfw_error_callback(int, const char* desc) {
    std::cout << desc << "\n";
}
} // namespace

int main() {
    // init GLFW
    glfwSetErrorCallback((GLFWerrorfun)glfw_error_callback);
    if (!glfwInit()) {
        std::cout << "failed to glfwInit\n";
        return EXIT_FAILURE;
    }

    // create GLFW window
#ifdef WIN32
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    GLFWwindow* window = glfwCreateWindow(640, 480, "Rendering Techniques Demo", nullptr, nullptr);
    if (!window) {
        std::cout << "failed to glfwCreateWindow\n";
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "failed to gladLoadGLLoader\n";
        return EXIT_FAILURE;
    }

    // init GUI
    gui::init(window);

    std::cout << "start main loop\n";
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        gui::new_frame();
        glViewport(0, 0, 640, 480);
        glClearColor(1.f, 0.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();

        glfwSwapBuffers(window);
    }
    std::cout << "finish main loop\n";

    gui::terminate();
    glfwTerminate();

    system("PAUSE");
    return EXIT_SUCCESS;
}
