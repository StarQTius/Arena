#pragma once

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/component/common.hpp>
#include <arena/environment.hpp>

#include <box2d/b2_body.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <tl/expected.hpp>

namespace arena {
namespace component {

struct Stackable {
  entt::entity next;
};

Expected<Stackable> make_stackable(arena::Environment &, entt::entity);

} // namespace component
} // namespace arena

template <> struct arena_component_info<arena::component::Stackable> {
  void on_storing(arena::component::Stackable &stackable, arena::Environment &environment) {
    b2Body *&body_p = environment.try_get<b2Body *>(environment.entity(stackable).value()).value();
    body_p->SetEnabled(false);

    auto next_entity = stackable.next;
    if (next_entity != entt::null) {
      b2Body *&next_body_p = environment.try_get<b2Body *>(next_entity).value();
      next_body_p->SetEnabled(true);
    }
  }
};
