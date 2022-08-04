#pragma once

#include "arena/component/common.hpp"
#include <cstddef>
#include <unordered_set>

#include <entt/entity/entity.hpp>

#include <arena/arena.hpp> // IWYU pragma: export
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace arena {
namespace stw {
namespace component {

enum class CupColor { RED, GREEN };

} // namespace component

namespace entity {

struct Cup {
  length_t x, y;
  stw::component::CupColor color;
};

entt::entity create(Environment &, const Cup &);

} // namespace entity

namespace component {

struct CupGrabber {
  std::unordered_set<entt::entity> storage;
  std::size_t capacity;

  explicit CupGrabber(std::size_t capacity) : storage{}, capacity{capacity} {}

  Expected<> grab(Environment &, entt::entity);
  Expected<> drop(Environment &, const entity::Cup &);
  std::size_t get_count(Environment &, CupColor) const;
};

} // namespace component
} // namespace stw
} // namespace arena

template <> struct arena_component_info<arena::stw::component::CupColor> {};

template <> struct arena_component_info<arena::stw::component::CupGrabber> {};
