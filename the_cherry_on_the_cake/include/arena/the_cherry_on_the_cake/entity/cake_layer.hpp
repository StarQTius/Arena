#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/the_cherry_on_the_cake/component/flavor.hpp>

namespace arena {
namespace coc {
namespace entity {

struct CakeLayer {
  length_t x, y;
  component::Flavor flavor;
  std::size_t stack;
};

entt::entity create(Environment &, const CakeLayer &);

} // namespace entity
} // namespace coc
} // namespace arena
