#pragma once

#include <cstddef>
#include <unordered_set>

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {

namespace component {
namespace c21 {

enum class CupColor { RED, GREEN };

constexpr inline auto arena_component_info(CupColor *) {
  struct {
  } component_info;

  return component_info;
}

} // namespace c21
} // namespace component

namespace entity {
namespace c21 {

struct Cup {
  length_t x, y;
  component::c21::CupColor color;
};

entt::entity create(Environment &, const Cup &);

} // namespace c21
} // namespace entity

namespace component {
namespace c21 {

struct CupGrabber {
  std::unordered_set<entt::entity> storage;
  std::size_t capacity;

  explicit CupGrabber(std::size_t capacity) : storage{}, capacity{capacity} {}

  Expected<> grab(Environment &, entt::entity);
  Expected<> drop(Environment &, const entity::c21::Cup &);
  std::size_t get_count(Environment &, c21::CupColor) const;
};

constexpr inline auto arena_component_info(CupGrabber *) {
  struct {
  } component_info;

  return component_info;
}

} // namespace c21
} // namespace component
} // namespace arena
