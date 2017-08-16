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
gui::Gui gui_;

void glfw_error_callback(int, const char* desc) {
  RT_LOG(error, "GLFW Error: {}", desc);
}

void render_drawlists(ImDrawData* draw_data) {
  gui_.render_drawlists(draw_data);
}

void mouse_button_callback(GLFWwindow*, int button, int action, int mods) {
  gui_.on_mouse_button(button, action, mods);
}

void scroll_callback(GLFWwindow*, double xoffset, double yoffset) {
  gui_.on_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow*, int key, int scancode, int action, int mods) {
  gui_.on_key(key, scancode, action, mods);
}

void char_callback(GLFWwindow*, unsigned int c) {
  gui_.on_char(c);
}
}  // namespace

int main() {
  // init GLFW
  glfwSetErrorCallback((GLFWerrorfun)glfw_error_callback);
  if (!glfwInit()) {
    RT_LOG(error, "failed to glfwInit");
    return EXIT_FAILURE;
  }

  // create GLFW window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  GLFWwindow* window = glfwCreateWindow(1280, 720, "Rendering Techniques Demo",
                                        nullptr, nullptr);
  if (!window) {
    RT_LOG(error, "failed to glfwCreateWindow");
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
    RT_LOG(error, "failed to glewInit");
    return EXIT_FAILURE;
  }

  RT_LOG(info, "start main");
  {
    // init GUI
    gui_.init(window, render_drawlists);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    // init application
    Application app;
    app.init();

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      gui_.new_frame();

      // clear frontbuffer
      glViewport(0, 0, 1280, 720);
      glDisable(GL_SCISSOR_TEST);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDepthMask(GL_TRUE);
      glClearColor(0.f, 0.f, 0.f, 0.f);
      glClearDepthf(1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      app.update();

      ImGui::Render();

      glfwSwapBuffers(window);
    }
  }
  RT_LOG(info, "finish main");

  gui_.terminate();
  glfwTerminate();

#ifdef WIN32
  system("PAUSE");
#else
// system("read -p \"Press Enter key to continue...\"");
#endif
  return EXIT_SUCCESS;
}
