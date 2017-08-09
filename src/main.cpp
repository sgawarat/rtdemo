#include <cstdlib>
#include <GL/glew.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rtdemo/gui.hpp>
#include <rtdemo/logging.hpp>
#include <rtdemo/application.hpp>

using namespace rtdemo;

namespace {
void glfw_error_callback(int, const char* desc) {
    RT_LOG_DEBUG("GLFW Error: {}", desc);
}
} // namespace

int main() {
    // init GLFW
    glfwSetErrorCallback((GLFWerrorfun)glfw_error_callback);
    if (!glfwInit()) {
        RT_LOG_DEBUG("failed to glfwInit");
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
        RT_LOG_DEBUG("failed to glfwCreateWindow");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1);
    glfwSwapInterval(0);

#ifdef WIN32
	freopen("CONOUT$", "w", stdout);
#endif

	// init GLEW
    if (glewInit() != GLEW_OK) {
        RT_LOG_DEBUG("failed to glewInit");
        return EXIT_FAILURE;
    }

    RT_LOG_DEBUG("start main");
    {
        // init GUI
        gui::init(window);

        // init application
        Application app;
        app.init();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            gui::new_frame();

            // clear frontbuffer
            glViewport(0, 0, 1280, 720);
            glDisable(GL_SCISSOR_TEST);
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClearDepthf(1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            app.update();

            ImGui::Render();

            glfwSwapBuffers(window);
        }
    }
    RT_LOG_DEBUG("finish main");

    gui::terminate();
    glfwTerminate();

#ifdef WIN32
    system("PAUSE");
#else
    // system("read -p \"Press Enter key to continue...\"");
#endif
    return EXIT_SUCCESS;
}
