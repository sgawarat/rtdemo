#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#define RT_LOG(v, ...) ::rtdemo::logging::get_logger()->v(__VA_ARGS__)

namespace rtdemo {
namespace logging {
std::shared_ptr<spdlog::logger> get_logger();
}  // namespace logging
}  // namespace rtdemo
