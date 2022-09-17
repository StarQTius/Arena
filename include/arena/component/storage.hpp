#pragma once

#include <cstddef>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/body.hpp>
#include <arena/component/common.hpp>
#include <arena/environment.hpp>

namespace arena {
namespace component {
namespace detail {

struct ownership_t {
  entt::entity owner;
};

struct Storage_impl {
  std::size_t m_count, m_capacity;

  Expected<> store(Expected<entt::entity>, Environment &, entt::entity);
  Expected<> remove(Expected<entt::entity>, Environment &, entt::entity);
};

} // namespace detail

template <Component Component_T> void on_storing(Component_T &&component, Environment &environment) {
  auto info = component_info(ARENA_FWD(component));

  ARENA_IF_LEGAL(info.on_storing(ARENA_FWD(component), environment);)
}

template <Component Component_T> void on_removal(Component_T &&component, Environment &environment) {
  auto info = component_info(ARENA_FWD(component));

  ARENA_IF_LEGAL(info.on_removal(ARENA_FWD(component), environment);)
}

template <Component... Component_Ts> class Storage : detail::Storage_impl {
  using impl_t = detail::Storage_impl;

public:
  explicit Storage(std::size_t capacity) : impl_t{.m_count = 0, .m_capacity = capacity} {}

  std::size_t count() const { return m_count; }
  std::size_t capacity() const { return m_capacity; }

  auto owned(Environment &environment) const {
    using namespace ltl;

    auto view = environment.view<Component_Ts..., detail::ownership_t>();
    auto this_entity = environment.entity(*this).value_or(entt::null);

    return view | filter([&environment, this_entity](auto entity) {
             return this_entity != entt::null && environment.get<detail::ownership_t>(entity).owner == this_entity;
           });
  }

  auto view(Environment &environment) const { return environment.view<Component_Ts...>(); }

  Expected<> store(Environment &environment, entt::entity entity) {
    using enum Error;

    ARENA_ASSERT(environment.all_of<Component_Ts...>(entity), NOT_ATTACHED);

    return impl_t::store(environment.entity(*this), environment, entity).and_then([&]() {
      (on_storing(environment.get<Component_Ts>(entity), environment), ...);
      return expected();
    });
  }

  Expected<> remove(Environment &environment, entt::entity entity) {
    using enum Error;

    ARENA_ASSERT(environment.all_of<Component_Ts...>(entity), NOT_ATTACHED);

    return impl_t::remove(environment.entity(*this), environment, entity).and_then([&]() {
      (on_removal(environment.get<Component_Ts>(entity), environment), ...);
      return expected();
    });
  }

  Expected<entt::entity> pick(Environment &environment, length_t x, length_t y) {
    using namespace ltl;

    auto is_hitting = [&](auto &&tuple) {
      auto *body_p = std::get<b2Body *&>(tuple);

      if (!body_p->IsEnabled())
        return false;

      for (auto *fixture_p = body_p->GetFixtureList(); fixture_p != nullptr; fixture_p = fixture_p->GetNext()) {
        if (fixture_p->TestPoint(b2Vec2{box2d_number(x), box2d_number(y)}))
          return true;
      }

      return false;
    };

    auto range = environment.view<b2Body *>().each() | filter(is_hitting);
    auto begin_it = begin(range), end_it = end(range);

    if (begin_it == end_it)
      return expected(entt::entity{entt::null});

    auto found_entity = std::get<entt::entity>(*begin_it);
    return store(environment, found_entity).transform([&]() { return found_entity; });
  }
};

} // namespace component
} // namespace arena

template <arena::Component... Component_Ts> struct arena_component_info<arena::component::Storage<Component_Ts...>> {};

template <> struct arena_component_info<arena::component::detail::ownership_t> {};
