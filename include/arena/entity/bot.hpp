#pragma once

#include <box2d/b2_shape.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <pybind11/pybind11.h>

#include <arena/component/body.hpp>

namespace arena {
namespace entity {

struct Bot {
  distance_t x, y;
  mass_t mass;
  pybind11::function logic;
};

entt::entity create(b2World &, entt::registry &, const Bot &, const b2Shape &);
} // namespace entity
} // namespace arena
