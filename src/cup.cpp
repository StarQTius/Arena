#include <arena/2021/cup.hpp>

#include <box2d/b2_circle_shape.h>

using namespace units::isq::si::literals;

namespace {

constexpr auto cup_mass = 200_q_g;
const auto cup_shape = arena::component::make_circle_shape(3_q_cm);

} // namespace

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const entity::Cup &def) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {def.x.number(), def.y.number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&cup_shape, compute_shape_density(cup_shape, cup_mass).number());

  auto self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);
  registry.emplace<component::Color>(self, def.color);

  return self;
}
