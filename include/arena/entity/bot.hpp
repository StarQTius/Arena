#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/pytypes.h>

#include <arena/physics.hpp>

namespace arena {
namespace entity {

struct Bot {
  length_t x, y;
  mass_t mass;
  pybind11::function logic;
};

entt::entity create(b2World &, entt::registry &, const Bot &);

} // namespace entity
} // namespace arena
