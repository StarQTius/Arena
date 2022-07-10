#include <arena/entity/bot.hpp>

#include <cmath>
#include <type_traits>

#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <units/isq/si/length.h>

#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/physics.hpp>

//
// Definitions
//

using namespace units::isq::si::literals;

namespace {

const auto bot_shape = arena::component::make_circle_shape(1200_q_mm / (2 * M_PI));

} // namespace

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const Bot &def) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {units::quantity_cast<si::metre>(def.x).number(),
                       units::quantity_cast<si::metre>(def.y).number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&bot_shape, compute_shape_density(bot_shape, def.mass).number());

  entt::entity self = registry.create();
  registry.emplace<b2Body *>(self, body_ptr);
  registry.emplace<component::PyHost>(self, def.logic);

  return self;
}
