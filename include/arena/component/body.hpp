#pragma once

#include <arena/arena.hpp> // IWYU pragma: export

#include <box2d/b2_circle_shape.h>
#include <entt/entity/registry.hpp>

#include <arena/physics.hpp>

// IWYU pragma: begin_exports
#include <arena/component/init_guard.hpp>
#include <box2d/b2_body.h>
// IWYU pragma: end_exports

namespace arena {

template <> struct init_guard<b2Body *> { static void init(entt::registry &); };

namespace component {

// Convenience function to create Box2D shapes
b2CircleShape make_circle_shape(length_t);

} // namespace component
} // namespace arena
