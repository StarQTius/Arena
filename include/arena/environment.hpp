#pragma once

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>

namespace arena {

// Physics engine and ECS registry bundle
// The primary reason for bundling these two together is to make sure the world is destroyed after the registry.
struct Environment {
  Environment() : world{{0, 0}} {}

  b2World world;
  entt::registry registry;
};

} // namespace arena
