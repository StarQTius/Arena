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
  arena::Expected<> on_storing(arena::component::Stackable &stackable, arena::Environment &environment) {
    return environment.entity(stackable)
        .and_then([&](auto entity) { return environment.try_get<b2Body *>(entity); })
        .transform([&](b2Body *body_p) {
          body_p->SetEnabled(false);
          auto next_entity = stackable.next;
          if (next_entity != entt::null)
            environment.try_get<b2Body *>(next_entity).transform([&](b2Body *body_p) { body_p->SetEnabled(true); });
        });
  }
};
