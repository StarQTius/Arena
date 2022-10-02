#include "arena/component/storage.hpp"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <ltl/functional.h>
#include <ltl/operator.h>
#include <tl/expected.hpp>

#include "arena/environment.hpp"

using namespace arena;

Expected<> arena::component::detail::Storage_impl::store(Expected<entt::entity> this_entity_xp,
                                                         Environment &environment, entt::entity target) {
  using enum Error;

  ARENA_ASSERT(m_count < m_capacity, STORAGE_FULL);

  return this_entity_xp
      .and_then([&](auto this_entity) {
        if (!environment.all_of<ownership_t>(target)) {
          environment.attach(target, ownership_t{this_entity});
          return expected();
        } else {
          auto &ownership = environment.get<ownership_t>(target);
          return ownership.owner == entt::null ? ownership.owner = this_entity, expected() : unexpected(ALREADY_OWNED);
        }
      })
      .transform([&]() { m_count++; });
}

Expected<> arena::component::detail::Storage_impl::remove(Expected<entt::entity> this_entity_xp,
                                                          Environment &environment, entt::entity target) {
  using namespace ltl;
  using enum Error;

  return expected(this_entity_xp, environment.try_get<ownership_t>(target))
      .and_then(unzip(_((this_entity, ownership), (ownership.owner == this_entity ? ownership.owner = entt::null,
                                                   expected()                     : unexpected(NOT_OWNED)))))
      .transform([&]() { m_count--; });
}
