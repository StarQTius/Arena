#pragma once

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

#include "def.hpp"

namespace arena {
namespace component {
ARENA_C21_INLINE namespace c21 {

  enum class CupColor { RED, GREEN };

} // namespace c21
} // namespace component

namespace entity {
ARENA_C21_INLINE namespace c21 {

  struct Cup {
    box2d_distance_t x, y;
    component::CupColor color;
  };

  entt::entity create(b2World &, entt::registry &, const Cup &);

} // namespace c21
} // namespace entity

namespace component {
ARENA_C21_INLINE namespace c21 {

  struct CupGrabber {
    std::unordered_set<entt::entity> storage;
    size_t storage_size;

    bool grab(Environment &environment, entt::entity target) {
      auto &&[body_ptr, cup_color] = environment.registry.try_get<BodyPtr, CupColor>(target);
      if (body_ptr && storage.size() < storage_size) {
        storage.insert(target);
        (*body_ptr)->SetEnabled(false);
        return true;
      } else {
        return false;
      }
    }

    bool drop(Environment &environment, const entity::Cup &cup) {
      auto is_same_color = [&](auto entity) { return environment.registry.get<CupColor>(entity) == cup.color; };
      auto cup_entity_it = std::find_if(storage.begin(), storage.end(), is_same_color);
      if (cup_entity_it != storage.end()) {
        auto &body_ptr = environment.registry.get<BodyPtr>(*cup_entity_it);
        body_ptr->SetEnabled(true);
        body_ptr->SetTransform({cup.x.number(), cup.y.number()}, body_ptr->GetAngle());
        storage.erase(cup_entity_it);
        return true;
      } else {
        return false;
      }
    }
  };

} // namespace c21
} // namespace component
} // namespace arena
