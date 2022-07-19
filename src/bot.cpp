
#include <cmath>
#include <type_traits>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <units/isq/si/length.h>

#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/entity/bot.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

//
// Definitions
//

using namespace arena;

namespace {

using namespace units::isq::si::literals;

const auto bot_shape = component::make_circle_shape(1200_q_mm / (2 * M_PI));

} // namespace

entt::entity arena::entity::create(Environment &environment, const Bot &def) {
  auto entity = environment.create();

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {box2d_number(def.x), box2d_number(def.y)};

  auto *body_p = environment.attach(entity, body_def);
  body_p->CreateFixture(&bot_shape, box2d_number(compute_shape_density(bot_shape, def.mass)));

  environment.attach(entity, component::PyHost{def.logic});

  return entity;
}
