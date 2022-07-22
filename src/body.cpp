#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/sigh.hpp>
#include <units/isq/si/length.h>

#include <arena/component/body.hpp>
#include <arena/physics.hpp>

using namespace arena;

namespace {

void destroy_body(entt::registry &registry, entt::entity entity) {
  get_world(registry).DestroyBody(registry.get<b2Body *>(entity));
}

} // namespace

void arena::component::body_info_t::init(entt::registry &registry) {
  registry.on_destroy<b2Body *>().connect<destroy_body>();
}

b2CircleShape arena::component::make_circle_shape(length_t radius) {
  using namespace units::isq;

  b2CircleShape shape;
  shape.m_p = {0, 0};
  shape.m_radius = units::quantity_cast<si::metre>(radius).number();

  return shape;
}

b2Body *arena_make_component(entt::registry &registry, const b2BodyDef &body_def) {
  return get_world(registry).CreateBody(&body_def);
}
