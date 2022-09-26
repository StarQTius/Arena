#pragma once

#include <entt/entity/registry.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace component {

struct Ray {
  length_t x, y, range;
  angle_t angle;

  Expected<length_t> cast(arena::Environment &) const;
};

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Ray> { static void init(entt::registry &); };
