#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/environment.hpp>

namespace arena {
namespace component {

class Stackable {
  friend Expected<Stackable> make_stackable(arena::Environment &, entt::entity);

public:
  Expected<> stack(Environment &, entt::entity);
  Expected<> unstack(Environment &environment);
  bool is_top() const { return m_is_top; }
  Expected<std::vector<entt::entity>> range(Environment &) const;

private:
  Stackable() : m_next{entt::null}, m_is_top{true} {}

  entt::entity m_next;
  bool m_is_top;
};

Expected<Stackable> make_stackable(arena::Environment &, entt::entity);

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Stackable> {
  arena::Expected<> on_storing(arena::component::Stackable &stackable, arena::Environment &environment) {
    return stackable.unstack(environment);
  }
};
