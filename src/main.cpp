#include <rtdemo/logging.hpp>
#include <rtdemo/application.hpp>

using namespace rtdemo;

int main() {
  // 初期化
  if (!Logger::get().init(spdlog::level::trace)) return EXIT_FAILURE;
  if (!Application::get().init(1280, 720)) return EXIT_FAILURE;

  // メインループ
  while (Application::get().update()) {
    std::this_thread::yield();
  }

  // 破棄
  Application::get().terminate();
  Logger::get().terminate();

  return EXIT_SUCCESS;
}
