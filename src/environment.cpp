#include <thread>

#include <box2d/b2_world.h>
#include <chrono>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/process/scheduler.hpp>
#include <entt/signal/delegate.hpp>
#include <units/isq/si/time.h>

#include <arena/draw.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace arena;

entt::entity arena::create(Environment &environment) { return environment.m_registry.create(); }

arena::Environment::Environment() : m_world_p{new b2World{{0, 0}}}, m_renderer{renderer_scale} {
  m_registry.ctx().emplace<b2World &>(*m_world_p);
  m_world_p->SetDebugDraw(&m_renderer);
  m_renderer.SetFlags(m_renderer.e_shapeBit);
}

arena::Environment::Environment(const entt::delegate<process_t> &upkeep) : Environment{} { m_scheduler.attach(upkeep); }

void arena::Environment::step(duration_t timestep) {
  m_scheduler.update(timestep, this);

  m_world_p->Step(timestep.number(), velocity_iterations, position_iterations);

  if (m_renderer) {
    m_world_p->DebugDraw();
    m_renderer.show();
    std::this_thread::sleep_for(units::quantity_cast<units::isq::si::second>(timestep).number() *
                                std::chrono::seconds{1});
  }
}

PyGameDrawer &arena::Environment::renderer() { return m_renderer; };
