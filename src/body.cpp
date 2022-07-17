#include <arena/component/body.hpp>
#include <entt/entity/entity.hpp>
#include <entt/signal/sigh.hpp>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <units/isq/si/length.h>

#include <arena/physics.hpp>

namespace {

void destroy_body(entt::registry &registry, entt::entity entity) {
  registry.ctx().at<b2World &>().DestroyBody(registry.get<b2Body *>(entity));
}

} // namespace

void arena::init_guard<b2Body *>::init(entt::registry &registry) {
  registry.on_destroy<b2Body *>().connect<destroy_body>();
}

b2CircleShape arena::component::make_circle_shape(length_t radius) {
  using namespace units::isq;

  b2CircleShape shape;
  shape.m_p = {0, 0};
  shape.m_radius = units::quantity_cast<si::metre>(radius).number();

  return shape;
}
