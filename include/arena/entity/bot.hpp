#pragma once

#include <cstddef>

#include <box2d/b2_shape.h>
#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/pytypes.h>

#include <arena/physics.hpp>

namespace arena {
namespace entity {

struct Bot {
  distance_t x, y;
  mass_t mass;
  pybind11::function logic;
  std::size_t cup_storage_size;
};

entt::entity create(b2World &, entt::registry &, const Bot &, const b2Shape &);
} // namespace entity
} // namespace arena
