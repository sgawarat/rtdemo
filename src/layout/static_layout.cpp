#include <rtdemo/layout/static_layout.hpp>
#include <rtdemo/managed.hpp>

namespace rtdemo {
// RT_MANAGED_LAYOUT_INSTACE(layout, StaticLayout);
namespace {
::rtdemo::ManagedLayout<layout::StaticLayout>
    MANAGED_LAYOUT_INSTANCE_StaticLayout{"StaticLayout"};
}

namespace layout {
const uint64_t StaticLayout::ID = 1000;
}  // namespace layout
}  // namespace rtdemo
