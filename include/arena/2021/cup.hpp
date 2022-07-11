#pragma once

#include <cstddef>
#include <unordered_set>

#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace component {
namespace c21 {

enum class CupColor { RED, GREEN };

} // namespace c21
} // namespace component

namespace entity {
namespace c21 {

struct Cup {
  length_t x, y;
  component::c21::CupColor color;
};

entt::entity create(b2World &, entt::registry &, const Cup &);

} // namespace c21
} // namespace entity

namespace component {
namespace c21 {

struct CupGrabber {
  std::unordered_set<entt::entity> storage;
  std::size_t capacity;

  explicit CupGrabber(std::size_t capacity) : storage{}, capacity{capacity} {}

  bool grab(Environment &environment, entt::entity target);
  bool drop(Environment &environment, const entity::c21::Cup &cup);
  std::size_t get_count(Environment &environment, c21::CupColor) const;
};

} // namespace c21
} // namespace component
} // namespace arena
