#pragma once

#include <box2d/b2_body.h>
#include <box2d/b2_chain_shape.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>

#include <arena/component/body.hpp>

namespace arena {
namespace entity {

struct Field {
  distance_t width, height;
};

entt::entity create(b2World &, entt::registry &, const Field &);

} // namespace entity
} // namespace arena
