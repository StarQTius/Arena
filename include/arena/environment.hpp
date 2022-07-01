#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <utility>

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <units/isq/si/time.h>

#include <arena/draw.hpp>
#include <arena/physics.hpp>

namespace {

// ADL enabler
[[maybe_unused]] void create(){};

} // namespace

namespace arena {

constexpr precision_t renderer_scale = 200;

// Physics engine and ECS registry bundle
// The primary reason for bundling these two together is to make sure the world is destroyed after the registry.
struct Environment {
  constexpr static int32_t velocity_iterations = 8;
  constexpr static int32_t position_iterations = 3;

  b2World world;
  entt::registry registry;
  PyGameDrawer renderer;
  std::function<void(Environment &)> upkeep;

  explicit Environment(std::invocable<Environment &> auto &&upkeep)
      : world{{0, 0}}, renderer{renderer_scale}, upkeep{std::forward<decltype(upkeep)>(upkeep)} {
    world.SetDebugDraw(&renderer);
    renderer.SetFlags(renderer.e_shapeBit);
  }

  Environment(const Environment &) = delete;
  Environment &operator=(const Environment &) = delete;
  Environment(Environment &&) = delete;
  Environment &operator=(Environment &&) = delete;

  template <typename... Args> auto create(Args &&...args) {
    using ::create;
    return create(world, registry, std::forward<Args>(args)...);
  }

  void step(duration_t timestep) {
    upkeep(*this);
    world.Step(timestep.number(), velocity_iterations, position_iterations);
    if (renderer) {
      world.DebugDraw();
      renderer.show();
      std::this_thread::sleep_for(units::quantity_cast<units::isq::si::second>(timestep).number() *
                                  std::chrono::seconds{1});
    }
  }
};

} // namespace arena
