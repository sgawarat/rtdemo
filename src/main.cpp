#include <cstdlib>
#include <imgui.h>
#include <rtdemo/gui.hpp>
#include <rtdemo/logging.hpp>
#include <rtdemo/scene/static_scene.hpp>
#include <rtdemo/tech/forward_shading.hpp>

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
    glfwSwapInterval(1);

#ifdef WIN32
	freopen("CONOUT$", "w", stdout);
#endif

	// init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        RT_LOG_DEBUG("failed to gladLoadGLLoader");
        return EXIT_FAILURE;
    }

    RT_LOG_DEBUG("start main");
    {
        // init GUI
        gui::init(window);

        // init scene
        scene::StaticScene scene;
        scene.init();

        // init tech
        tech::ForwardShading shading;
        shading.init();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            gui::new_frame();

            ImGui::Text("hello world");

            glViewport(0, 0, 1280, 720);
            glDisable(GL_SCISSOR_TEST);
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClearDepthf(1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shading.apply();
            scene.apply();
            scene.draw();

            ImGui::Render();

            glfwSwapBuffers(window);
        }
    }
    RT_LOG_DEBUG("finish main");

    gui::terminate();
    glfwTerminate();

    system("PAUSE");
    return EXIT_SUCCESS;
}
