#pragma once

#include <box2d/b2_world.h>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <arena/component/body.hpp>

#include "def.hpp"

namespace arena {
namespace component {
ARENA_C21_INLINE namespace c21 {

  enum class Color { RED, GREEN };

} // namespace c21
} // namespace component

namespace entity {
ARENA_C21_INLINE namespace c21 {

  struct Cup {
    box2d_distance_t x, y;
    component::Color color;
  };

  entt::entity create(b2World &, entt::registry &, const Cup &);

} // namespace c21
} // namespace entity
} // namespace arena
