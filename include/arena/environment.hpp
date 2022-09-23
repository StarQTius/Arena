#pragma once

#include <utility>

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/helper.hpp> // IWYU pragma: keep
#include <entt/entity/registry.hpp>
#include <entt/process/scheduler.hpp>
#include <entt/signal/delegate.hpp>
#include <entt/signal/dispatcher.hpp>
#include <ltl/Tuple.h>
#include <ltl/operator.h>
#include <ltl/tuple_algos.h>
#include <units/isq/si/time.h>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/body.hpp>
#include <arena/draw.hpp>
#include <arena/physics.hpp>
#include <arena/traits/invocable.hpp>

#define DescribingEntity DescribingEntity

namespace arena {

class Environment;

using process_t = void(duration_t, void *, std::function<void()>, std::function<void()>);

constexpr precision_t renderer_scale = 200;

template <typename... Args>
concept DescribingEntity = requires(Environment &environment, Args &&...args) {
  { create(environment, ARENA_FWD(args)...) } -> std::convertible_to<entt::entity>;
};

entt::entity create(Environment &);

// Physics engine and ECS registry bundle
// The primary reason for bundling these two together is to make sure the world is destroyed after the registry.
class Environment {
  friend entt::entity create(Environment &);

public:
  constexpr static int32_t velocity_iterations = 8;
  constexpr static int32_t position_iterations = 3;

  Environment();
  explicit Environment(const entt::delegate<process_t> &);

  void step(duration_t);
  PyGameDrawer &renderer();

  explicit Environment(std::invocable<duration_t, void *, void *, void *> auto &&upkeep) : Environment{} {
    m_scheduler.attach(ARENA_FWD(upkeep));
  }

  template <typename... Args>
  requires DescribingEntity<Args...>
  auto create(Args &&...args) {
    using arena::create;
    return create(*this, std::forward<Args>(args)...);
  }

  template <Component Component_T> decltype(auto) attach(entt::entity entity, Component_T &&component) {
    using component_t = std::remove_cvref_t<Component_T>;

    check_init_guard<Component_T>();
    m_registry.emplace<component_t>(entity, ARENA_FWD(component));

    return get<component_t>(entity);
  }

  template <Component Component_T, typename... Args>
  requires DescribingComponent<Component_T, Args...>
  auto &attach(entt::entity entity, Args &&...args) {
    using component_t = std::remove_cvref_t<Component_T>;

    check_init_guard<Component_T>();
    return m_registry.emplace<component_t>(entity, make_component<Component_T>(m_registry, entity, ARENA_FWD(args)...));
  }

  template <auto Function> void on_collision() { m_dispatcher.sink<CollisionBeginning>().connect<Function>(); }

  template <auto Function> void on_collision(auto &instance) {
    m_dispatcher.sink<CollisionBeginning>().connect<Function>(instance);
  }

  template <Component... Component_Ts, Component... Exclude_Ts> auto view(entt::exclude_t<Exclude_Ts...> exclude_list) {
    return m_registry.view<Component_Ts...>(exclude_list);
  }

  template <Component... Component_Ts> auto view() { return view<Component_Ts...>({}); }

  template <Component... Component_Ts> decltype(auto) get(entt::entity entity) {
    return m_registry.get<Component_Ts...>(entity);
  }

  template <Component... Component_Ts> auto try_get(entt::entity entity) {
    using namespace ltl;
    using enum Error;

    if constexpr (sizeof...(Component_Ts) == 1) {
      return entity != entt::null ? expected(m_registry.try_get<Component_Ts...>(entity), NOT_ATTACHED)
                                  : unexpected(NOT_ATTACHED);
    } else {
      tuple_t components_p{m_registry.try_get<Component_Ts>(entity)...};

      return entity != entt::null && components_p(_((... ps), (ps && ...)))
                 ? expected(transform_type(components_p, _((p), *p)))
                 : unexpected(NOT_ATTACHED);
    }
  }

  template <Component... Component_Ts> bool all_of(entt::entity entity) const {
    return m_registry.all_of<Component_Ts...>(entity);
  }

  template <Component Component_T> auto entity(const Component_T &component) {
    return expected(entt::to_entity(m_registry, component), Error::DANGLING_COMPONENT);
  }

  template <Component... Component_Ts> auto remove(entt::entity entity) {
    return m_registry.remove<Component_Ts...>(entity);
  }

private:
  template <Component Component_T> void check_init_guard() {
    // This type is bound to the function template instance, therefore it is different across distinct template
    // instantiations
    struct guard_t {};

    if constexpr (Initializable<Component_T>) {
      if (!m_registry.ctx().contains<guard_t>()) {
        init<Component_T>(m_registry);
        m_registry.ctx().emplace<guard_t>();
      }
    }
  }

  std::unique_ptr<b2World> m_world_p;
  entt::scheduler<duration_t> m_scheduler;
  entt::dispatcher m_dispatcher;
  PyGameDrawer m_renderer;
  entt::registry m_registry;
};

} // namespace arena
