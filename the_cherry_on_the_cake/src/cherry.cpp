#include "arena/the_cherry_on_the_cake/entity/cherry.hpp"

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

#include "arena/the_cherry_on_the_cake/component/cherry_like.hpp"
#include "arena/the_cherry_on_the_cake/component/flavor.hpp"

// IWYU pragma: no_include <units/isq/si/time.h>

namespace {

using namespace units::isq::si::literals;

auto cherry_shape = arena::component::make_circle_shape(22.5_q_mm);
constexpr auto cherry_mass = 1.83_q_g;
constexpr auto cherry_friction_force = 0_q_N;
constexpr auto cherry_friction_torque = 0_q_N_m_per_rad;

arena::Expected<> on_stack(arena::Environment &environment, arena::component::Stackable &stackable,
                           entt::entity target) {
  using namespace arena;

  using component::Stackable, coc::component::Flavor;
  using ltl::unzip;
  using enum Error;

  ARENA_ASSERT((environment.all_of<b2Body *, Stackable, Flavor>(target)), NOT_ATTACHED);

  return environment.entity(stackable).and_then([&](entt::entity entity) -> Expected<> {
    ARENA_PROPAGATE(expected(environment.try_get<b2Body *>(entity), environment.try_get<b2Body *>(target))
                        .transform(unzip([&](b2Body *body_p, b2Body *target_body_p) {
                          body_p->SetTransform(target_body_p->GetPosition(), 0);
                        })));
    environment.attach<b2WeldJoint *>(entity, target);
    return expected();
  });
}

arena::Expected<> on_unstack(arena::Environment &environment, arena::component::Stackable &stackable) {
  using namespace arena;

  using component::Stackable, coc::component::Flavor;
  using ltl::unzip;
  using enum Error;

  return environment.entity(stackable).transform(
      [&](entt::entity entity) { environment.remove<b2WeldJoint *>(entity); });
}

} // namespace

entt::entity arena::coc::entity::create(Environment &environment, const Cherry &def) {
  auto entity = environment.create();

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {box2d_number(def.x), box2d_number(def.y)};
  auto *body_p = environment.attach<b2Body *>(entity, body_def);
  body_p->CreateFixture(&cherry_shape, box2d_number(compute_shape_density(cherry_shape, cherry_mass)));

  environment.attach<b2FrictionJoint *>(entity, cherry_friction_force, cherry_friction_torque);
  environment.attach(entity, component::CherryLike{});
  environment.attach(entity, arena::component::make_stackable(environment, entt::null, on_stack, on_unstack).value());

  return entity;
}
