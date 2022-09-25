#pragma once

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>
#include <entt/entity/registry.hpp>
#include <units/isq/dimensions/force.h>
#include <units/isq/dimensions/torque.h>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/physics.hpp>

// IWYU pragma: begin_exports
#include <arena/component/common.hpp>
#include <box2d/b2_body.h>
#include <box2d/b2_friction_joint.h>
#include <box2d/b2_weld_joint.h>
// IWYU pragma: end_exports

namespace arena {

class Environment;

struct CollisionBeginning {
  entt::entity entity_a, entity_b;
  entt::registry *registry_p;
};

entt::entity get_entity(b2Body *);
b2Body *get_ground_p(entt::registry &);

namespace component {

b2CircleShape make_circle_shape(length_t);

} // namespace component
} // namespace arena

template <> struct arena_component_info<b2Body *> {
  static void init(entt::registry &);
  static b2Body *make(entt::registry &, entt::entity, const b2BodyDef &);

  static arena::Expected<> on_storing(b2Body *body_p, arena::Environment &) {
    body_p->SetEnabled(false);

    return arena::expected();
  }

  static arena::Expected<> on_removal(b2Body *body_p, arena::Environment &) {
    body_p->SetEnabled(true);

    return arena::expected();
  }
};

template <> struct arena_component_info<b2FrictionJoint *> {
  static void init(entt::registry &);
  static b2FrictionJoint *make(entt::registry &, entt::entity, const b2FrictionJointDef &);

  template <units::isq::Force Force_T, units::isq::Torque Torque_T>
  static b2FrictionJoint *make(entt::registry &registry, entt::entity entity, Force_T force, Torque_T torque) {
    using namespace arena;

    b2FrictionJointDef def;
    auto &world = arena::get_world(registry);

    b2Body *body_p = nullptr;
    if (!registry.all_of<b2Body *>(entity)) {
      b2BodyDef body_def;
      body_p = registry.emplace<b2Body *>(entity, world.CreateBody(&body_def));
    } else {
      body_p = registry.get<b2Body *>(entity);
    }

    def.Initialize(body_p, get_ground_p(registry), {0, 0});
    def.maxForce = box2d_number(force);
    def.maxTorque = box2d_number(torque);
    return reinterpret_cast<b2FrictionJoint *>(world.CreateJoint(&def));
  }
};

template <> struct arena_component_info<b2WeldJoint *> {
  static void init(entt::registry &);
  static b2WeldJoint *make(entt::registry &, entt::entity, entt::entity);
};
