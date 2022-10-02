#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace coc {
namespace entity {

struct Cherry {
  length_t x, y;
};

entt::entity create(Environment &, const Cherry &);

} // namespace entity
} // namespace coc
} // namespace arena
