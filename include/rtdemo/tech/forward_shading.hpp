#pragma once

#include <rtdemo/garie.hpp>

namespace rtdemo {
namespace tech {
class ForwardShading {
public:
    bool init();

    void apply() const noexcept;

private:
    garie::Program prog_;
};
}  // namespace tech
}  // namespace rtdemo
