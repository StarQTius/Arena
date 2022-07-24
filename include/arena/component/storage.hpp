#pragma once

#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <cstddef>

namespace arena {
namespace component {
namespace detail {

struct ownership_t {
   entt::entity owner;
};

constexpr auto arena_component_info(ownership_t *) {
  struct {} info;
  return info;
}

struct Storage_impl {
  std::size_t m_count, m_capacity;
  
  Expected<> store(Expected<entt::entity>, Environment&, entt::entity);
  Expected<> remove(Expected<entt::entity>,Environment&, entt::entity);
};

} // namespace detail

template<Component Component_T>
void on_storing(Component_T &&component, Environment &environment) {
  auto info = arena_component_info(&component);

  ARENA_IF_LEGAL(
    info.on_storing(ARENA_FWD(component), environment);
  )
}

template<Component Component_T>
void on_removal(Component_T &&component, Environment &environment) {
  auto info = arena_component_info(&component);

  ARENA_IF_LEGAL(
    info.on_removal(ARENA_FWD(component), environment);
  )
}

template<Component... Component_Ts>
class Storage : detail::Storage_impl {
  using impl_t = detail::Storage_impl;

public:
  explicit Storage(std::size_t capacity) : impl_t{.m_count = 0, .m_capacity = capacity}  {}

  std::size_t count() const {
    return m_count;
  }
  
  std::size_t capacity() const {
    return m_capacity;
  }

  auto owned(Environment &environment) const {
    using namespace ltl;

    auto view = environment.view<Component_Ts..., detail::ownership_t>();
    auto this_entity = environment.entity(*this).value_or(entt::null);
    
    return view | filter([&environment, this_entity](auto entity) {
      return this_entity != entt::null && environment.get<detail::ownership_t>(entity).owner == this_entity;
    });
  }

  auto view(Environment &environment) const {
    return environment.view<Component_Ts...>();
  }

  Expected<> store(Environment &environment, entt::entity entity) {
    using enum Error;

    ARENA_ASSERT(environment.all_of<Component_Ts...>(entity), NOT_ATTACHED);

    return impl_t::store(environment.entity(*this), environment, entity)
      .and_then([&]() {
        (on_storing(environment.get<Component_Ts>(entity), environment), ...);
        return expected();
      }
    );
  }

  Expected<> remove(Environment &environment, entt::entity entity) {
    using enum Error;

    ARENA_ASSERT(environment.all_of<Component_Ts...>(entity), NOT_ATTACHED);

    return impl_t::remove(environment.entity(*this), environment, entity)
      .and_then([&]() {
        (on_removal(environment.get<Component_Ts>(entity), environment), ...);
        return expected();
      }
    );
  }
};

template<Component... Component_Ts>
auto arena_component_info(Storage<Component_Ts...> *) {
  struct {} info;
  return info;
}

} // namespace component
} // namespace arena
