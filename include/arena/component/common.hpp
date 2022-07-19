#pragma once

#include <type_traits>

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/concept.hpp>

#define HasInitGuard HasInitGuard
#define DescribingComponent DescribingComponent

namespace arena {

template <typename> struct init_guard; // IWYU pragma: keep

template <typename T>
concept Component = requires(std::remove_cvref_t<T> x) {
  { arena_component_info(&x) } -> Empty;
};

template <typename Component_T>
concept HasInitGuard = Component<Component_T> && requires(entt::registry &registry) {
  init_guard<std::remove_cvref_t<Component_T>>::init(registry);
};

template <typename... Args>
concept DescribingComponent = requires(entt::registry &registry, Args &&...args) {
  { arena_make_component(registry, ARENA_FWD(args)...) } -> Component;
};

template <HasInitGuard Component_T> void init(init_guard<Component_T>, entt::registry &registry) {
  init_guard<Component_T>::init(registry);
}

inline auto &get_world(entt::registry &registry) { return registry.ctx().at<b2World &>(); }

} // namespace arena
