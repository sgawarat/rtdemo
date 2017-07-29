#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        std::cout << "failed to glfwInit\n";
        return EXIT_FAILURE;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Rendering Techniques Demo", nullptr, nullptr);
    if (!window) {
        std::cout << "failed to glfwCreateWindow\n";
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cout << "failed to gladLoadGL\n";
        return EXIT_FAILURE;
    }

    std::cout << "start main loop";
    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.f, 0.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    std::cout << "finish main loop\n";

    glfwTerminate();

    system("PAUSE");
    return EXIT_SUCCESS;
}
