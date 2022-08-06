#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace entity {

struct Bot {
  length_t x, y;
  mass_t mass;
};

entt::entity create(Environment &, const Bot &);

} // namespace entity
} // namespace arena
