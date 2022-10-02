#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace entity {

struct Field {
  length_t width, height;
};

entt::entity create(Environment &, const Field &);

} // namespace entity
} // namespace arena
