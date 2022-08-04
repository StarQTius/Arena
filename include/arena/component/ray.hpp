#pragma once

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace component {

struct Ray {
  length_t x, y, range;
  angle_t angle;
};

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Ray> {};
