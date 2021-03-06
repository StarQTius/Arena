#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

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
