#include <type_traits>
#include <unordered_set>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <ltl/algos.h>
#include <ltl/functional.h>
#include <ltl/optional.h>
#include <tl/expected.hpp>
#include <units/generic/angle.h>
#include <units/isq/si/force.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/torque.h>

#include <arena/2021/cup.hpp>
#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace arena;

namespace {

using namespace units::isq::si::literals;

const auto cup_shape = arena::component::make_circle_shape(36_q_mm);

auto is_same_color(Environment &environment, stw::component::CupColor color, entt::entity entity) {
  using namespace ltl;

  auto check = [&](auto *, auto entity_color) { return entity_color == color; };

  return environment.try_get<b2Body *, stw::component::CupColor>(entity).transform(unzip(check)).value_or(false);
}

} // namespace

entt::entity arena::stw::entity::create(Environment &environment, const entity::Cup &def) {
  auto entity = environment.create();

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {box2d_number(def.x), box2d_number(def.y)};

  auto *body_p = environment.attach<b2Body *>(entity, body_def);
  body_p->CreateFixture(&cup_shape, box2d_number(compute_shape_density(cup_shape, 10_q_g)));
  environment.attach<b2FrictionJoint *>(entity, 0.001_q_N, 0.001_q_N_m_per_rad);
  environment.attach(entity, def.color);

  return entity;
}

Expected<> arena::stw::component::CupGrabber::grab(Environment &environment, entt::entity target) {
  using namespace ltl;
  using enum Error;

  ARENA_ASSERT(storage.size() < capacity, STORAGE_FULL);

  auto disable_body = [&](auto *body_p, auto) {
    storage.insert(target);
    body_p->SetEnabled(false);
  };

  return environment.try_get<b2Body *, CupColor>(target).map(unzip(disable_body));
}

Expected<> arena::stw::component::CupGrabber::drop(Environment &environment, const entity::Cup &cup) {
  using namespace ltl;
  using namespace units::literals;
  using namespace units::angle_references;
  using enum Error;

  auto enable_body = [&](auto entity) {
    auto *body_p = environment.get<b2Body *>(entity);
    body_p->SetEnabled(true);
    body_p->SetTransform({box2d_number(cup.x), box2d_number(cup.y)}, box2d_number(0_q_rad));
    storage.erase(entity);
  };

  return expected(find_if_value(storage, curry(is_same_color)(std::ref(environment), cup.color)), NOT_IN_STORAGE)
      .map(enable_body);
}

std::size_t arena::stw::component::CupGrabber::get_count(Environment &environment, CupColor color) const {
  using namespace ltl;

  return count_if(storage, curry(is_same_color)(std::ref(environment), color));
}
