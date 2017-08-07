#pragma once

#include <string>
#include <rtdemo/garie.hpp>

namespace rtdemo {
namespace tech {
class ForwardShading {
public:
    bool init();

    void apply() const noexcept;

    explicit operator bool() const noexcept {
        return info_log_.empty();
    }

    const std::string& info_log() const noexcept {
        return info_log_;
    }

private:
    garie::Program prog_;
    std::string info_log_;
};
}  // namespace tech
}  // namespace rtdemo
