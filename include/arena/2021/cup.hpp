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
    std::unordered_map<CupColor, size_t> storage;
    size_t storage_size;

    bool grab(Environment &environment, entt::entity target) {
      auto &&[body_ptr, cup_color] = environment.registry.try_get<BodyPtr, CupColor>(target);
      if (body_ptr && cup_color && std::accumulate(storage.begin(), storage.end(), 0u, [](auto lhs, auto rhs) {
                                     return lhs + rhs.second;
                                   }) < storage_size) {
        storage[*cup_color]++;
        environment.registry.destroy(target);
        return true;
      } else {
        return false;
      }
    }

    bool drop(Environment &environment, const entity::Cup &cup) {
      if (storage[cup.color] > 0) {
        storage[cup.color]--;
        environment.create(cup);
        return true;
      } else {
        return false;
      }
    }
  };

} // namespace c21
} // namespace component
} // namespace arena
