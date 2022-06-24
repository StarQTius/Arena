#include <arena/component/body.hpp>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <units/isq/si/length.h>

#include <arena/physics.hpp>

b2CircleShape arena::component::make_circle_shape(distance_t radius) {
  using namespace units::isq;

  b2CircleShape shape;
  shape.m_p = {0, 0};
  shape.m_radius = units::quantity_cast<si::metre>(radius).number();

  return shape;
}
