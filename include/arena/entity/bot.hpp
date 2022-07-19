#pragma once

#include <entt/entity/entity.hpp>
#include <pybind11/pytypes.h>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace entity {

struct Bot {
  length_t x, y;
  mass_t mass;
  pybind11::function logic;
};

entt::entity create(Environment &, const Bot &);

} // namespace entity
} // namespace arena
