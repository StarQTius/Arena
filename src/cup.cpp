#include <arena/2021/cup.hpp>

#include <type_traits>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/registry.hpp>
#include <ltl/algos.h>
#include <ltl/functional.h>
#include <ltl/optional.h>
#include <tl/expected.hpp>
#include <units/generic/angle.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace arena;

namespace {

using namespace units::isq::si::literals;

constexpr auto cup_mass = 10_q_g;
constexpr auto cup_damping = 100_q_s;
const auto cup_shape = arena::component::make_circle_shape(36_q_mm);

auto is_same_color(Environment &environment, component::c21::CupColor color, entt::entity entity) {
  using namespace ltl;

  auto check = [&](auto *, auto entity_color) { return entity_color == color; };

  return environment.try_get<b2Body *, component::c21::CupColor>(entity).transform(unzip(check)).value_or(false);
}

} // namespace

entt::entity arena::entity::c21::create(Environment &environment, const entity::c21::Cup &def) {
  auto entity = environment.create();

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {box2d_number(def.x), box2d_number(def.y)};

  auto *body_p = environment.attach(entity, body_def);
  body_p->CreateFixture(&cup_shape, box2d_number(compute_shape_density(cup_shape, cup_mass)));
  body_p->SetLinearDamping(box2d_number(cup_damping));
  body_p->SetAngularDamping(box2d_number(cup_damping));

  environment.attach(entity, def.color);

  return entity;
}

Expected<> arena::component::c21::CupGrabber::grab(Environment &environment, entt::entity target) {
  using namespace ltl;
  using enum Error;

  ARENA_ASSERT(storage.size() < capacity, STORAGE_FULL);

  auto disable_body = [&](auto *body_p, auto) {
    storage.insert(target);
    body_p->SetEnabled(false);
  };

  return environment.try_get<b2Body *, CupColor>(target).map(unzip(disable_body));
}

Expected<> arena::component::c21::CupGrabber::drop(Environment &environment, const entity::c21::Cup &cup) {
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

std::size_t arena::component::c21::CupGrabber::get_count(Environment &environment, CupColor color) const {
  using namespace ltl;

  return count_if(storage, curry(is_same_color)(std::ref(environment), color));
}
