#include <rtdemo/logging.hpp>
#include <memory>

namespace rtdemo {
namespace {
std::shared_ptr<spdlog::logger> console;
}  // namespace

std::shared_ptr<spdlog::logger> get_logger() {
    if (!console) {
        console = spdlog::stdout_logger_mt("console");
    }
    return console;
}
}  // namespace rtdemo
