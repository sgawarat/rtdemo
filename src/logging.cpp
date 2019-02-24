#include <rtdemo/logging.hpp>
#include <cstdlib>
#include <spdlog/sinks/stdout_sinks.h>

namespace rtdemo {
Logger& Logger::get() noexcept {
  static Logger self;
  return self;
}

bool Logger::init(spdlog::level::level_enum level) {
#ifdef WIN32
  // Windowsアプリケーションでもコマンドプロンプトを表示させる
  static FILE* stream = nullptr;
  freopen_s(&stream, "CONOUT$", "w", stdout);
#endif

  logger_ = spdlog::stdout_logger_mt("console");
  if (!logger_) return false;

  logger_->set_level(level);

  return true;
}

void Logger::terminate() {
  // ログを確認するためにコンソールへのキー入力を要求する
  if (max_level_ >= spdlog::level::warn) {
#ifdef WIN32
    system("PAUSE");
#else
    system("read -p \"Press Enter key to continue...\"");
#endif
  }
}
}  // namespace rtdemo
