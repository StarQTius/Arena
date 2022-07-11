#include <arena/2021/cup.hpp>

#include <algorithm>
#include <type_traits>

#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace units::isq::si::literals;

namespace {

constexpr auto cup_mass = 10_q_g;
constexpr auto cup_damping = 100_q_s;
const auto cup_shape = arena::component::make_circle_shape(36_q_mm);

} // namespace

entt::entity arena::entity::c21::create(b2World &world, entt::registry &registry, const entity::c21::Cup &def) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {def.x.number(), def.y.number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&cup_shape, compute_shape_density(cup_shape, cup_mass).number());
  body_ptr->SetLinearDamping(duration_t{cup_damping}.number());
  body_ptr->SetAngularDamping(duration_t{cup_damping}.number());

  auto self = registry.create();
  registry.emplace<b2Body *>(self, body_ptr);
  registry.emplace<component::c21::CupColor>(self, def.color);

  return self;
}

bool arena::component::c21::CupGrabber::grab(Environment &environment, entt::entity target) {
  auto &&[body_ptr, cup_color] = environment.registry.try_get<b2Body *, CupColor>(target);
  if (body_ptr && storage.size() < capacity) {
    storage.insert(target);
    (*body_ptr)->SetEnabled(false);
    return true;
  } else {
    return false;
  }
}

bool arena::component::c21::CupGrabber::drop(Environment &environment, const entity::c21::Cup &cup) {
  auto is_same_color = [&](auto entity) { return environment.registry.get<CupColor>(entity) == cup.color; };
  auto cup_entity_it = std::find_if(storage.begin(), storage.end(), is_same_color);
  if (cup_entity_it != storage.end()) {
    auto &body_ptr = environment.registry.get<b2Body *>(*cup_entity_it);
    body_ptr->SetEnabled(true);
    body_ptr->SetTransform({cup.x.number(), cup.y.number()}, body_ptr->GetAngle());
    storage.erase(cup_entity_it);
    return true;
  } else {
    return false;
  }
}

std::size_t arena::component::c21::CupGrabber::get_count(Environment &environment, CupColor color) const {
  return std::ranges::count_if(storage,
                               [&](auto entity) { return environment.registry.get<CupColor>(entity) == color; });
}
