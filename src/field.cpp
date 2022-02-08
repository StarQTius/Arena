#include <arena/entity/field.hpp>

#include <arena/component/body.hpp>
#include <units/quantity_cast.h>

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const arena::entity::Field &def) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.position = {0, 0};

  b2ChainShape boundaries;
  auto width = units::quantity_cast<si::metre>(def.width).number();
  auto height = units::quantity_cast<si::metre>(def.height).number();
  b2Vec2 boundaries_chains[] = {
      {width / 2, height / 2}, {width / 2, -height / 2}, {-width / 2, -height / 2}, {-width / 2, height / 2}};
  boundaries.CreateLoop(boundaries_chains, 4);

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(&boundaries, 0);

  auto self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);

  return self;
}
