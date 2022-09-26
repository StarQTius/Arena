#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <arena/physics.hpp>

namespace arena {

struct CollisionBeginning {
  entt::entity entity_a, entity_b;
  entt::registry *registry_p;
};

struct RayFired {
  entt::entity entity;
  length_t x, y, distance;
  angle_t angle;
};

} // namespace arena
