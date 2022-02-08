#pragma once

#include <utility>

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>

#include <arena/binding/fetcher.hpp>
#include <arena/physics.hpp>

namespace {

// ADL enabler
[[maybe_unused]] void create(){};

} // namespace

namespace arena {

// Physics engine and ECS registry bundle
// The primary reason for bundling these two together is to make sure the world is destroyed after the registry.
struct Environment {
  constexpr static int32_t velocity_iterations = 8;
  constexpr static int32_t position_iterations = 3;

  explicit Environment(std::invocable<entt::registry &> auto &&upkeep)
      : world{{0, 0}}, upkeep{std::forward<decltype(upkeep)>(upkeep)} {}

  b2World world;
  entt::registry registry;
  std::function<void(entt::registry &)> upkeep;

  template <typename... Args> auto create(Args &&...args) {
    using ::create;
    return create(world, registry, std::forward<Args>(args)...);
  }

  void step(box2d_time_t timestep) {
    upkeep(registry);
    world.Step(timestep.number(), velocity_iterations, position_iterations);
  }
};

} // namespace arena
