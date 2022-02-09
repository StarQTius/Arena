#pragma once

#include <chrono>
#include <optional>
#include <thread>
#include <utility>

#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>

#include <SFMLDebugDraw.h>
#include <arena/binding/fetcher.hpp>
#include <arena/physics.hpp>

namespace {

// ADL enabler
[[maybe_unused]] void create(){};

} // namespace

namespace arena {

constexpr struct with_rendering_t {
} with_rendering;

// Physics engine and ECS registry bundle
// The primary reason for bundling these two together is to make sure the world is destroyed after the registry.
struct Environment {
  constexpr static int32_t velocity_iterations = 8;
  constexpr static int32_t position_iterations = 3;

  explicit Environment(std::invocable<entt::registry &> auto &&upkeep)
      : world{{0, 0}}, upkeep{std::forward<decltype(upkeep)>(upkeep)} {}

  explicit Environment(std::invocable<entt::registry &> auto &&upkeep, with_rendering_t)
      : world{{0, 0}}, upkeep{std::forward<decltype(upkeep)>(upkeep)},
        m_render_window{std::in_place, sf::VideoMode{1000, 1000}, "Arena"}, m_drawer{*m_render_window} {
    m_drawer->SetFlags(b2Draw::e_shapeBit);
    world.SetDebugDraw(&m_drawer.value());
  }

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
    if (m_drawer) {
      world.DebugDraw();
      m_render_window->display();
      std::this_thread::sleep_for(units::quantity_cast<units::isq::si::second>(timestep).number() *
                                  std::chrono::seconds{1});
      m_render_window->clear();
    }
  }

private:
  std::optional<sf::RenderWindow> m_render_window;
  std::optional<SFMLDebugDraw> m_drawer;
};

} // namespace arena
