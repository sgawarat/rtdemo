#include <rtdemo/application.hpp>
#include <chrono>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gsl/gsl>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>
#include <rtdemo/logging.hpp>
#include <rtdemo/gui.hpp>
#include <rtdemo/scene.hpp>
#include <rtdemo/technique.hpp>
#include <rtdemo/util.hpp>

namespace rtdemo {
Application& Application::get() noexcept {
  static Application self;
  return self;
}

bool Application::init(size_t screen_width, size_t screen_height) {
  // 初期化に失敗した場合にterminateを呼ぶようにする
  bool succeeded = false;
  auto _ = gsl::finally([&, this] {
    if (!succeeded) terminate();
  });

  // GLFWを初期化する
  glfwSetErrorCallback([](int, const char* desc) {
    RT_ERROR("GLFW Error: {}", desc);
  });
  if (!glfwInit()) {
    RT_ERROR("GLFWの初期化に失敗した");
    return false;
  }

  // GLFWウィンドウを生成する
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  window_ = glfwCreateWindow(static_cast<int>(screen_width), static_cast<int>(screen_height), "rtdemo", nullptr, nullptr);
  if (!window_) {
    RT_ERROR("GLFWウィンドウの生成に失敗した");
    return false;
  }
  glfwMakeContextCurrent(window_);
  // glfwSwapInterval(1);  // VSyncオン
  glfwSwapInterval(0);  // VSyncオフ

  // GLEWを初期化する
  if (glewInit() != GLEW_OK) {
    RT_ERROR("GLEWの初期化に失敗した");
    return false;
  }

  // GUIを初期化する
  if (!Gui::get().init(window_)) {
    RT_ERROR("GUIの初期化に失敗した");
    return false;
  }
  glfwSetMouseButtonCallback(window_, [](GLFWwindow*, int button, int action, int mods) {
    Gui::get().on_mouse_button(button, action, mods);
  });
  glfwSetScrollCallback(window_, [](GLFWwindow*, double x, double y) {
    Gui::get().on_scroll(x, y);
  });
  glfwSetKeyCallback(window_, [](GLFWwindow*, int key, int scancode, int action, int mods) {
    Gui::get().on_key(key, scancode, action, mods);
  });
  glfwSetCharCallback(window_, [](GLFWwindow*, unsigned int c) {
    Gui::get().on_char(c);
  });


  int w = 0;
  int h = 0;
  glfwGetFramebufferSize(window_, &w, &h);
  screen_width_ = static_cast<uint32_t>(w);
  screen_height_ = static_cast<uint32_t>(h);
  current_scene_ = scene_map_.end();
  current_technique_ = technique_map_.end();

  succeeded = true;  // 初期化に成功した
  return true;
}

void Application::terminate() {
  Gui::get().terminate();
  glfwTerminate();
  window_ = nullptr;
  screen_width_ = 0;
  screen_height_ = 0;
  current_scene_ = scene_map_.end();
  current_technique_ = technique_map_.end();
}

bool Application::update() {
  // ウィンドウが閉じていれば、更新できない
  if (glfwWindowShouldClose(window_)) return false;

  // OSのイベントを処理する
  glfwPollEvents();

  // GUI開始
  Gui::get().new_frame();

  // シーン名の一覧を表示するコンボボックスを描画する
  {
    const char* preview_value = "-----";  // コンボボックスが閉じているときに表示する文字列
    if (current_scene_ != scene_map_.end()) {
      preview_value = current_scene_->first.c_str();
    }
    if (ImGui::BeginCombo("current scene", preview_value)) {
      // コンボボックスが開いていれば、選択可能な要素を設定する
      auto last = scene_map_.end();
      for (auto iter = scene_map_.begin(); iter != last; ++iter) {
        const bool selected = iter == current_scene_;
        if (ImGui::Selectable(iter->first.c_str(), &selected)) {
          // 要素が選択されれば、シーンを切り替える
          if (current_scene_ != scene_map_.end() && current_scene_->second) {
            current_scene_->second->invalidate();
          }
          current_scene_ = iter;
          if (current_scene_->second) {
            current_scene_->second->restore();
          }
        }
        if (selected) {
          // コンボボックスを開いたとき、すでに選択されている要素にフォーカスする
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }

  // テクニック名の一覧を表示するコンボボックスを描画する
  {
    const char* preview_value = "-----";  // コンボボックスが閉じているときに表示する文字列
    if (current_technique_ != technique_map_.end()) {
      preview_value = current_technique_->first.c_str();
    }
    if (ImGui::BeginCombo("current technique", preview_value)) {
      // コンボボックスが開いていれば、選択可能な要素を設定する
      auto last = technique_map_.end();
      for (auto iter = technique_map_.begin(); iter != last; ++iter) {
        const bool selected = iter == current_technique_;
        if (ImGui::Selectable(iter->first.c_str(), &selected)) {
          // 要素が選択されれば、シーンを切り替える
          if (current_technique_ != technique_map_.end() && current_technique_->second) {
            current_technique_->second->invalidate();
          }
          current_technique_ = iter;
          if (current_technique_->second) {
            current_technique_->second->restore();
          }
        }
        if (selected) {
          // コンボボックスを開いたとき、すでに選択されている要素にフォーカスする
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }

  // FPSを計算して表示する
  {
    using namespace std::chrono_literals;

    using Seconds = std::chrono::duration<double>;
    using Milliseconds = std::chrono::duration<double, std::milli>;
    using Nanoseconds = std::chrono::duration<double, std::nano>;
    using Clock = std::chrono::high_resolution_clock;

    // 経過時間を計測する
    static Nanoseconds elapsed_time;  // 経過時間
    static auto last_tp = Clock::now();  // 前回計測したときの時間
    auto now_tp = Clock::now();
    elapsed_time = std::chrono::duration_cast<Nanoseconds>(now_tp - last_tp);
    last_tp = now_tp;

    // 平均FPSを計算する
    static auto checkpoint_tp = Clock::now() + 1s;  // この時刻を超えたら平均を計算する
    static Nanoseconds sum_elapsed_time;  // 経過時間の合計
    static size_t elapsed_time_count = 0;  // 合計した経過時間の数
    static Nanoseconds ave_elapsed_time;  // 平均経過時間
    static double ave_fps = 0.0;  // 平均FPS
    sum_elapsed_time += elapsed_time;
    elapsed_time_count++;
    if (now_tp >= checkpoint_tp) {
      ave_elapsed_time = sum_elapsed_time / elapsed_time_count;
      ave_fps = 1.0 / std::chrono::duration_cast<Seconds>(ave_elapsed_time).count();
      sum_elapsed_time = {};
      elapsed_time_count = 0;
      checkpoint_tp = now_tp + 1s;
    }

    ImGui::Text("Ave. Time:%5.3lf[ms]", std::chrono::duration_cast<Milliseconds>(ave_elapsed_time));
    ImGui::Text("Ave. FPS :%5.3lf[fps]", ave_fps);
  }

  // 状態を更新する
  if (current_scene_ != scene_map_.end() &&
      current_technique_ != technique_map_.end() &&
      current_scene_->second &&
      current_technique_->second) {
    current_scene_->second->update_gui();
    current_technique_->second->update_gui();

    current_scene_->second->update();
    current_technique_->second->update();

    current_technique_->second->apply(*current_scene_->second);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    util::screen_viewport().apply();
    util::clear({0.f, 0.f, 0.f, 0.f}, 1.f);
  }

  // GUIを描画する
  Gui::get().render();

  // レンダリング結果をウィンドウに表示する
  glfwSwapBuffers(window_);

  return true;
}

bool Application::insert_scene(std::string name, std::shared_ptr<Scene> scene) {
  auto iter = scene_map_.find(name);
  if (iter != scene_map_.end()) return false;  // 同名への上書きはできない

  scene_map_.emplace(std::move(name), std::move(scene));
  return true;
}

bool Application::insert_technique(std::string name, std::shared_ptr<Technique> technique) {
  auto iter = technique_map_.find(name);
  if (iter != technique_map_.end()) return false;  // 同名への上書きはできない

  technique_map_.emplace(std::move(name), std::move(technique));
  return true;
}
}  // namespace rtdemo
