#include <rtdemo/logging.hpp>

namespace rtdemo {
namespace logging {
namespace {
std::shared_ptr<spdlog::logger> console;
}  // namespace

std::shared_ptr<spdlog::logger> get_logger() {
  if (!console) {
    console = spdlog::stdout_logger_mt("console");
    console->set_level(spdlog::level::trace);
  }
  return console;
}
}  // namespace logging
}  // namespace rtdemo
