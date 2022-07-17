#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <entt/entity/registry.hpp>

namespace arena {

template <typename> struct init_guard; // IWYU pragma: keep

template <typename Component_T> void init(init_guard<Component_T>, entt::registry &registry) {
  init_guard<Component_T>::init(registry);
}

} // namespace arena
