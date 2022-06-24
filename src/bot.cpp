#include <arena/entity/bot.hpp>

#include <type_traits>

#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <box2d/b2_shape.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <units/isq/si/length.h>

#include <arena/2021/cup.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/physics.hpp>

//
// Definitions
//

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const Bot &def, const b2Shape &shape) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {units::quantity_cast<si::metre>(def.x).number(),
                       units::quantity_cast<si::metre>(def.y).number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&shape, compute_shape_density(shape, def.mass).number());

  entt::entity self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);
  registry.emplace<component::PyHost>(self, def.logic);
  registry.emplace<component::c21::CupGrabber>(
      self, component::c21::CupGrabber{.storage = {}, .storage_size = def.cup_storage_size});

  return self;
}
