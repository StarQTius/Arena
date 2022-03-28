#include <arena/entity/bot.hpp>

#include <units/quantity_cast.h>

#include <arena/2021/cup.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>

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

  auto self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);
  registry.emplace<component::PyHost>(self, def.logic);
  registry.emplace<component::CupGrabber>(
      self, component::CupGrabber{.storage = {{component::CupColor::RED, 0}, {component::CupColor::GREEN, 0}},
                                  .storage_size = def.cup_storage_size});

  return self;
}
