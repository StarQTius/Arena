#pragma once

#include <arena/physics.hpp>
#include <arena/arena.hpp> // IWYU pragma: export

namespace arena {
namespace component {

struct Ray {
  length_t x, y, range;
  angle_t angle;
};

constexpr auto arena_component_info(Ray *) {
  struct {
  } info;
  return info;
}

} // namespace component
} // namespace arena
