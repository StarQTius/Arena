#include <chrono>
#include <thread>

#include <box2d/b2_collision.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/process/scheduler.hpp>
#include <entt/signal/delegate.hpp>
#include <entt/signal/dispatcher.hpp>
#include <ltl/Tuple.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/draw.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/signal.hpp>

using namespace arena;

namespace {

auto get_entities(b2Contact &contact) {
  return ltl::tuple_t{get_entity(contact.GetFixtureA()->GetBody()), get_entity(contact.GetFixtureB()->GetBody())};
}

class ContactListener : public b2ContactListener {
public:
  ContactListener(entt::registry &registry) : m_registry{registry} {}

  virtual void BeginContact(b2Contact *contacts) {
    auto &dispatcher = get_dispatcher(m_registry);

    for (auto &contact = *contacts; contacts; contacts = contacts->GetNext()) {
      auto &&[entity_a, entity_b] = get_entities(contact);
      dispatcher.trigger(CollisionBeginning{.entity_a = entity_a, .entity_b = entity_b, .registry_p = &m_registry});
    }
  }

  virtual void EndContact(b2Contact *) {}
  virtual void PreSolve(b2Contact *, const b2Manifold *) {}
  virtual void PostSolve(b2Contact *, const b2ContactImpulse *) {}

private:
  entt::registry &m_registry;
};

} // namespace

entt::entity arena::create(Environment &environment) { return environment.m_registry.create(); }

arena::Environment::Environment() : m_world_p{new b2World{{0, 0}}}, m_renderer{renderer_scale} {
  m_registry.ctx().emplace<b2World &>(*m_world_p);
  m_registry.ctx().emplace<entt::dispatcher &>(m_dispatcher);

  m_world_p->SetDebugDraw(&m_renderer);
  m_world_p->SetContactListener(new ContactListener{m_registry});

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
