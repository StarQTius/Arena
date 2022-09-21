#pragma once

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/environment.hpp>

namespace arena {
namespace component {

class Stackable;

} // namespace component

using on_stack_t = Expected<>(Environment &, component::Stackable &, entt::entity);
using on_unstack_t = Expected<>(Environment &, component::Stackable &);

namespace component {

class Stackable {
  friend Expected<Stackable> make_stackable(arena::Environment &, entt::entity, on_stack_t *, on_unstack_t *);

public:
  Expected<> stack(Environment &, entt::entity);
  Expected<> unstack(Environment &environment);
  entt::entity next() const { return m_next; }
  bool is_top() const { return m_is_top; }
  Expected<std::vector<entt::entity>> range(Environment &) const;

private:
  Stackable() : Stackable{nullptr, nullptr} {}
  explicit Stackable(on_stack_t *on_stack, on_unstack_t *on_unstack)
      : m_next{entt::null}, m_is_top{true}, m_on_stack{on_stack}, m_on_unstack{on_unstack} {}

  entt::entity m_next;
  bool m_is_top;
  on_stack_t *m_on_stack;
  on_unstack_t *m_on_unstack;
};

Expected<Stackable> make_stackable(arena::Environment &, entt::entity);
Expected<Stackable> make_stackable(arena::Environment &, entt::entity, on_stack_t *on_stack, on_unstack_t *on_unstack);

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Stackable> {
  arena::Expected<> on_storing(arena::component::Stackable &stackable, arena::Environment &environment) {
    return stackable.unstack(environment);
  }
};
