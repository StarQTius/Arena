#pragma once

#include <box2d/b2_circle_shape.h>

#include <arena/physics.hpp>

namespace arena {
namespace component {

// Convenience function to create Box2D shapes
b2CircleShape make_circle_shape(length_t);

} // namespace component
} // namespace arena
