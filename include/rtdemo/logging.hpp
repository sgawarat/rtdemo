#pragma once

#include <spdlog/spdlog.h>

#define RT_LOG_DEBUG(...) SPDLOG_DEBUG(rtdemo::logging::get_logger(), __VA_ARGS__)

namespace rtdemo {
namespace logging {
std::shared_ptr<spdlog::logger> get_logger();
}  // namespace logging
}  // namespace rtdemo
