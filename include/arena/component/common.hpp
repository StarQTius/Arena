#pragma once

#include <type_traits>

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/traits/type.hpp>

#if defined(ARENA_IWYU)
#define Initializable Initializable
#define DescribingComponent DescribingComponent
#define Component Component
#endif // ARENA_IWYU

template <typename> struct arena_component_info; // IWYU pragma: keep

namespace arena {

template <typename T>
concept Component = requires {
  {
    arena_component_info<std::remove_cvref_t<T>> {}
    } -> Empty;
};

template <typename Component_T>
concept Initializable = Component<Component_T> && requires(entt::registry &registry) {
  arena_component_info<Component_T>::init(registry);
};

template <typename Component_T, typename... Args>
concept DescribingComponent = requires(entt::registry &registry, entt::entity entity, Args &&...args) {
  arena_component_info<Component_T>::make(registry, entity, ARENA_FWD(args)...);
};

template <Initializable Component_T> void init(entt::registry &registry) {
  arena_component_info<Component_T>::init(registry);
}

template <Component Component_T, typename... Args>
requires DescribingComponent<Component_T, Args...> Component_T make_component(entt::registry &registry,
                                                                              entt::entity entity, Args &&...args) {
  return arena_component_info<Component_T>::make(registry, entity, ARENA_FWD(args)...);
}

inline auto &get_world(entt::registry &registry) { return registry.ctx().at<b2World &>(); }
inline auto &get_dispatcher(entt::registry &registry) { return registry.ctx().at<entt::dispatcher &>(); }

template <Component Component_T> constexpr Empty auto component_info(Component_T &&) {
  return arena_component_info<std::remove_cvref_t<Component_T>>{};
}

} // namespace arena
