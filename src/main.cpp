#include <cstdlib>
#ifdef WIN32
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>
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
  // GLFWを初期化する
  glfwSetErrorCallback((GLFWerrorfun)glfw_error_callback);
  if (!glfwInit()) {
    RT_LOG(error, "GLFWの初期化に失敗した");
    return EXIT_FAILURE;
  }

  // GLFWウィンドウを生成する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  GLFWwindow* window = glfwCreateWindow(1280, 720, "rtdemo",
                                        nullptr, nullptr);
  if (!window) {
    RT_LOG(error, "GLFWウィンドウの生成に失敗した");
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  // glfwSwapInterval(1);  // VSyncオン
  glfwSwapInterval(0);  // VSyncオフ

#ifdef WIN32
  // コンソールを表示する
  FILE* stream = nullptr;
  freopen_s(&stream, "CONOUT$", "w", stdout);
#endif

  // GLEWを初期化する
  if (glewInit() != GLEW_OK) {
    RT_LOG(error, "GLEWの初期化に失敗した");
    return EXIT_FAILURE;
  }

  RT_LOG(info, "=== 開始 ===");
  {
    // GUIを初期化する
    gui_.init(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    // アプリケーションを初期化する
    Application app;
    app.init();

    // メインループ
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      gui_.new_frame();

      app.update();

      gui_.render();

      glfwSwapBuffers(window);
    }
  }
  RT_LOG(info, "=== 終了 ===");

  gui_.terminate();
  glfwTerminate();

  // ログを確認するためにコンソールへのキー入力を要求する
#ifdef WIN32
  system("PAUSE");
#else
// system("read -p \"Press Enter key to continue...\"");
#endif
  return EXIT_SUCCESS;
}
