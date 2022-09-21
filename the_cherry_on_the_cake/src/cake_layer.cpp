#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <ltl/functional.h>
#include <tl/expected.hpp>
#include <units/isq/si/force.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/torque.h>

#include <arena/component/body.hpp>
#include <arena/component/stackable.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/the_cherry_on_the_cake/component/flavor.hpp>
#include <arena/the_cherry_on_the_cake/entity/cake_layer.hpp>

namespace {

using namespace units::isq::si::literals;

auto cake_layer_shape = arena::component::make_circle_shape(60_q_mm);
constexpr auto cake_layer_mass = 100_q_g;
constexpr auto cake_layer_friction_force = 0_q_N;
constexpr auto cake_layer_friction_torque = 0_q_N_m_per_rad;

arena::Expected<> on_stack(arena::Environment &environment, arena::component::Stackable &, entt::entity target) {
  using arena::component::Stackable, arena::coc::component::Flavor;
  using ltl::unzip;

  return environment.try_get<b2Body *, Stackable, Flavor>(target).transform(
      unzip([&](b2Body *next_body_p, auto &&...) { return next_body_p->SetEnabled(false); }));
}

arena::Expected<> on_unstack(arena::Environment &environment, arena::component::Stackable &stackable) {
  using arena::component::Stackable, arena::coc::component::Flavor;
  using ltl::unzip;

  return environment.try_get<b2Body *, Stackable, Flavor>(stackable.next())
      .transform(unzip([&](b2Body *next_body_p, auto &&...) { return next_body_p->SetEnabled(true); }));
}

} // namespace

entt::entity arena::coc::entity::create(Environment &environment, const CakeLayer &def) {
  entt::entity previous_entity = entt::null;

  for (std::size_t i = 0; i < def.stack; i++) {
    auto entity = environment.create();

    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {box2d_number(def.x), box2d_number(def.y)};
    auto *body_p = environment.attach<b2Body *>(entity, body_def);
    body_p->CreateFixture(&cake_layer_shape, box2d_number(compute_shape_density(cake_layer_shape, cake_layer_mass)));

    environment.attach<b2FrictionJoint *>(entity, cake_layer_friction_force, cake_layer_friction_torque);
    environment.attach(entity, def.flavor);
    environment.attach(entity,
                       arena::component::make_stackable(environment, previous_entity, on_stack, on_unstack).value());
    previous_entity = entity;
  }

  return previous_entity;
}
