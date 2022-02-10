#include <arena/2021/cup.hpp>

#include <box2d/b2_circle_shape.h>

using namespace units::isq::si::literals;

namespace {

constexpr auto cup_mass = 200_q_g;
constexpr auto cup_damping = 100_q_s;
const auto cup_shape = arena::component::make_circle_shape(3_q_cm);

} // namespace

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const entity::Cup &def) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {def.x.number(), def.y.number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&cup_shape, compute_shape_density(cup_shape, cup_mass).number());
  body_ptr->SetLinearDamping(box2d_time_t{cup_damping}.number());
  body_ptr->SetAngularDamping(box2d_time_t{cup_damping}.number());

  auto self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);
  registry.emplace<component::Color>(self, def.color);

  return self;
}
