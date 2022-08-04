#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_settings.h>
#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/sigh.hpp>
#include <units/isq/si/length.h>

#include <arena/component/body.hpp>
#include <arena/physics.hpp>

using namespace arena;

namespace {

struct ground_t {
  b2Body *body_p;
};

void write_userdata(entt::registry &registry, entt::entity entity) {
  registry.get<b2Body *>(entity)->GetUserData().pointer = static_cast<uintptr_t>(entity);
}

void destroy_body(entt::registry &registry, entt::entity entity) {
  get_world(registry).DestroyBody(registry.get<b2Body *>(entity));
}

} // namespace

entt::entity arena::get_entity(b2Body *body_p) { return static_cast<entt::entity>(body_p->GetUserData().pointer); }

b2Body *arena::get_ground_p(entt::registry &registry) { return registry.ctx().at<ground_t>().body_p; }

b2CircleShape arena::component::make_circle_shape(length_t radius) {
  using namespace units::isq;

  b2CircleShape shape;
  shape.m_p = {0, 0};
  shape.m_radius = units::quantity_cast<si::metre>(radius).number();

  return shape;
}

void arena_component_info<b2Body *>::init(entt::registry &registry) {
  registry.on_construct<b2Body *>().connect<write_userdata>();
  registry.on_destroy<b2Body *>().connect<destroy_body>();
}

b2Body *arena_component_info<b2Body *>::make(entt::registry &registry, entt::entity, const b2BodyDef &body_def) {
  return get_world(registry).CreateBody(&body_def);
}

void arena_component_info<b2FrictionJoint *>::init(entt::registry &registry) {
  b2BodyDef def;
  registry.ctx().emplace<ground_t>(get_world(registry).CreateBody(&def));
}

b2FrictionJoint *arena_component_info<b2FrictionJoint *>::make(entt::registry &registry, entt::entity,
                                                               const b2FrictionJointDef &friction_joint_def) {
  return static_cast<b2FrictionJoint *>(get_world(registry).CreateJoint(&friction_joint_def));
}
