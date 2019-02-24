#pragma once

#include <spdlog/spdlog.h>

#define RT_LOG(l, ...) ::rtdemo::Logger::get().log(::spdlog::level::l, __VA_ARGS__)
#define RT_DEBUG(...) RT_LOG(debug, __VA_ARGS__)
#define RT_WARN(...) RT_LOG(warn, __VA_ARGS__)
#define RT_ERROR(...) RT_LOG(err, __VA_ARGS__)

namespace rtdemo {
class Logger {
public:
  static Logger& get() noexcept;

  /**
   * @brief 初期化する
   * 
   * @param level 出力するログの最低レベル
   * @return true 成功した
   * @return false 失敗した
   */
  bool init(spdlog::level::level_enum level);

  /**
   * @brief 破棄する
   * 
   * warn以上のログが出力されると、確認のためのキー入力を要求する。
   */
  void terminate();

  /**
   * @brief ログレベルを確認して、ログを出力する
   * 
   * @tparam Args 引数の型
   * @param level ログレベル
   * @param fmt フォーマット文字列
   * @param args 引数
   */
  template <typename... Args>
  void log(spdlog::level::level_enum level, const char* fmt, Args&&... args) {
    if (level > max_level_) max_level_ = level;
    if (logger_) logger_->log(level, fmt, std::forward<Args>(args)...);
  }

private:
  spdlog::level::level_enum max_level_ = spdlog::level::trace;  ///< 出力されたログの中で最も大きなレベル
  std::shared_ptr<spdlog::logger> logger_;
};
}  // namespace rtdemo
