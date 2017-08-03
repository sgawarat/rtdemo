#include <iostream>
#include <cstdlib>
#include <rendering_techniques/stable.hpp>
#include <rendering_techniques/scene.hpp>
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Rendering Techniques Demo", nullptr, nullptr);
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

    scene::Scene scene;
    scene.load();

    std::cout << "start main loop\n";
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        gui::new_frame();

        ImGui::Text("hello world");

        glViewport(0, 0, 1280, 720);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClearDepthf(1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // scene.draw();

        ImGui::Render();

        glfwSwapBuffers(window);
    }
    std::cout << "finish main loop\n";

    gui::terminate();
    glfwTerminate();

    system("PAUSE");
    return EXIT_SUCCESS;
}
