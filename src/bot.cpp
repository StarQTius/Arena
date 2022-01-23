#include <arena/entity/bot.hpp>

#include <units/quantity_cast.h>

#include <arena/component/host.hpp>
#include <arena/component/physics.hpp>

using namespace units::isq::si::length_references;
using namespace units::isq::si::mass_references;

//
// Internals
//

using kilogram_per_metre_sqr = decltype(kg / (m * m))::unit;

// Compute the uniform density of a shape given its mass
static auto compute_shape_density(const b2Shape &shape, units::isq::Mass auto mass) {
  using namespace arena;

  b2MassData mass_data;
  shape.ComputeMass(&mass_data, 1);

  return mass / mass_t{mass_data.mass} * density_t{1};
}

//
// Definitions
//

entt::entity arena::entity::create(b2World &world, entt::registry &registry, const arena::entity::Bot &def,
                                   const b2Shape &shape) {
  using namespace units::isq;

  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = {units::quantity_cast<si::metre>(def.x).number(),
                       units::quantity_cast<si::metre>(def.y).number()};

  auto *body_ptr = world.CreateBody(&body_def);
  body_ptr->CreateFixture(
      &shape, units::quantity_cast<kilogram_per_metre_sqr>(compute_shape_density(shape, def.mass)).number());

  auto self = registry.create();
  registry.emplace<component::BodyPtr>(self, body_ptr);
  registry.emplace<component::PyHost>(self, def.logic);

  return self;
}
