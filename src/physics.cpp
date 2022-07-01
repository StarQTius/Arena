#include <arena/physics.hpp>

#include <box2d/b2_shape.h>

arena::density_t arena::compute_shape_density(const b2Shape &shape, mass_t mass) {
  b2MassData mass_data;
  shape.ComputeMass(&mass_data, 1);

  return mass / mass_t{mass_data.mass} * density_t{1};
}
