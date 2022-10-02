#include "arena/component/stackable.hpp"

#include <entt/entity/registry.hpp>
#include <ltl/functional.h>
#include <tl/expected.hpp>

arena::Expected<> arena::component::Stackable::stack(Environment &environment, entt::entity top) {
  using enum Error;

  ARENA_ASSERT(m_is_top && m_next == entt::null, ALREADY_STACKED);

  return environment.try_get<Stackable>(top).and_then([&](Stackable &top_stackable) -> Expected<> {
    ARENA_ASSERT(top_stackable.m_is_top, NOT_STACK_TOP);

    if (m_on_stack)
      ARENA_PROPAGATE(m_on_stack(environment, *this, top));

    m_next = top;
    top_stackable.m_is_top = false;

    return expected();
  });
}

arena::Expected<> arena::component::Stackable::unstack(Environment &environment) {
  using enum Error;

  ARENA_ASSERT(m_is_top, NOT_STACK_TOP);

  if (m_next == entt::null)
    return expected();

  return environment.try_get<Stackable>(m_next).and_then([&](Stackable &next_stackable) -> Expected<> {
    if (m_on_unstack)
      ARENA_PROPAGATE(m_on_unstack(environment, *this));

    m_next = entt::null;
    next_stackable.m_is_top = true;

    return expected();
  });
}

arena::Expected<std::vector<entt::entity>> arena::component::Stackable::range(arena::Environment &environment) const {
  return environment.entity(*this).and_then([&](entt::entity entity) -> Expected<std::vector<entt::entity>> {
    std::vector retval = {entity};

    while (retval.back() != entt::null) {
      ARENA_PROPAGATE(environment.try_get<Stackable>(retval.back()).transform([&](Stackable &stackable) {
        retval.push_back(stackable.m_next);
      }));
    }

    retval.pop_back();
    return expected(retval);
  });
}

arena::Expected<arena::component::Stackable> arena::component::make_stackable(Environment &environment,
                                                                              entt::entity top) {
  return make_stackable(environment, top, nullptr, nullptr);
}

arena::Expected<arena::component::Stackable> arena::component::make_stackable(Environment &environment,
                                                                              entt::entity top, on_stack_t on_stack,
                                                                              on_unstack_t on_unstack) {
  using ltl::unzip;
  using enum Error;

  Stackable retval{on_stack, on_unstack};
  retval.stack(environment, top);

  return retval;
}
