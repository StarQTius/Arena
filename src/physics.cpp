#include <arena/component/physics.hpp>

#include <box2d/b2_circle_shape.h>
#include <units/quantity_cast.h>

arena::box2d_density_t arena::compute_shape_density(const b2Shape &shape, mass_t mass) {
  b2MassData mass_data;
  shape.ComputeMass(&mass_data, 1);

  return mass / mass_t{mass_data.mass} * density_t{1};
}

b2CircleShape arena::component::make_circle_shape(distance_t radius) {
  using namespace units::isq;

  b2CircleShape shape;
  shape.m_p = {0, 0};
  shape.m_radius = units::quantity_cast<si::metre>(radius).number();

  return shape;
}
