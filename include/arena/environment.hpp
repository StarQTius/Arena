#pragma once

#include <chrono>
#include <thread>
#include <utility>

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <entt/process/scheduler.hpp>
#include <entt/signal/delegate.hpp>
#include <units/isq/si/time.h>

#include <arena/detail/forward.hpp>
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
  entt::scheduler<duration_t> scheduler;
  entt::registry registry;
  PyGameDrawer m_renderer;

  template <auto Upkeep>
  requires std::invocable<decltype(Upkeep), duration_t, Environment *, void (*)(), void (*)()>
  explicit Environment(entt::connect_arg_t<Upkeep> upkeep) : world{{0, 0}}, m_renderer{renderer_scale} {
    scheduler.attach(entt::delegate{upkeep});

    world.SetDebugDraw(&m_renderer);

    m_renderer.SetFlags(m_renderer.e_shapeBit);
  }

  explicit Environment(std::invocable<duration_t, Environment *, void (*)(), void (*)()> auto &&upkeep)
      : world{{0, 0}}, m_renderer{renderer_scale} {
    scheduler.attach(ARENA_FWD(upkeep));

    world.SetDebugDraw(&m_renderer);

    m_renderer.SetFlags(m_renderer.e_shapeBit);
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
    scheduler.update(timestep, this);

    world.Step(timestep.number(), velocity_iterations, position_iterations);

    if (m_renderer) {
      world.DebugDraw();
      m_renderer.show();
      std::this_thread::sleep_for(units::quantity_cast<units::isq::si::second>(timestep).number() *
                                  std::chrono::seconds{1});
    }
  }

  PyGameDrawer &renderer() { return m_renderer; };
};

} // namespace arena
