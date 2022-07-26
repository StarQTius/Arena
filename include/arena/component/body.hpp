#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <box2d/b2_circle_shape.h>
#include <entt/entity/registry.hpp>
#include <box2d/b2_fixture.h>

#include <arena/physics.hpp>

// IWYU pragma: begin_exports
#include <arena/component/common.hpp>
#include <box2d/b2_body.h>
// IWYU pragma: end_exports

namespace arena {

struct CollisionBeginning {
  entt::entity entity_a, entity_b;
  entt::registry *registry_p;
};

inline auto get_entity(b2Body *body_p) {
  return static_cast<entt::entity>(body_p->GetUserData().pointer);
}

namespace component {

struct body_info_t {
  static void init(entt::registry &);
};

// Convenience function to create Box2D shapes
b2CircleShape make_circle_shape(length_t);

} // namespace component
} // namespace arena

constexpr inline arena::component::body_info_t arena_component_info(b2Body **) {
  return {};
}

b2Body *arena_make_component(entt::registry &, const b2BodyDef &);
