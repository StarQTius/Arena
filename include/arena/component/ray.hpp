#pragma once

#include <entt/entity/entity.hpp>
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
  std::function<bool(entt::entity)> filter = [](auto &&) { return true; };

  Expected<length_t> cast(arena::Environment &) const;
  Expected<std::vector<length_t>> sweep(arena::Environment &, angle_t, std::size_t);
};

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Ray> { static void init(entt::registry &); };
