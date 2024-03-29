#include "arena/component/ray.hpp"

#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <ltl/Range/Value.h>
#include <ltl/functional.h>
#include <tl/expected.hpp>
#include <units/generic/angle.h>
#include <units/isq/si/length.h>

#include "arena/environment.hpp"
#include "arena/physics.hpp"
#include "arena/signal.hpp"
#include "arena/system/raycasting.hpp"

// IWYU pragma: no_include <units/isq/si/time.h>

using namespace arena;

namespace {

struct EventTrigger {
  entt::dispatcher *dispatcher_p;
};

} // namespace

void arena_component_info<component::Ray>::init(entt::registry &registry) {
  auto &dispatcher = registry.ctx().at<entt::dispatcher>();
  registry.ctx().emplace<EventTrigger>(EventTrigger{.dispatcher_p = &dispatcher});
}

Expected<length_t> arena::component::Ray::cast(arena::Environment &environment) const {
  using namespace units::isq::si::literals;
  using namespace units::literals;
  using ltl::unzip;

  return environment.entity(*this)
      .and_then([&](entt::entity entity) {
        return expected(expected(entity), environment.try_get<b2Body *>(entity), environment.try_ctx<EventTrigger>());
      })
      .transform(unzip([&](entt::entity entity, b2Body *body_p, EventTrigger &event_trigger) {
        auto length = system::cast(*this, body_p);
        event_trigger.dispatcher_p->trigger(
            RayFired{.entity = entity,
                     .x = x + 1_q_m * body_p->GetPosition().x / box2d_number(1_q_m),
                     .y = y + 1_q_m * body_p->GetPosition().y / box2d_number(1_q_m),
                     .distance = length,
                     .angle = angle + 1_q_rad * body_p->GetAngle() / box2d_number(1_q_rad)});

        return length;
      }));
}

Expected<std::vector<length_t>> arena::component::Ray::sweep(Environment &environment, angle_t fov,
                                                             std::size_t definition) {
  auto middle_angle = angle;

  std::vector<length_t> retval{};
  retval.reserve(definition);

  for (auto i : ltl::steppedValueRange((long)definition / -2, (long)definition / 2, 1l)) {
    angle = middle_angle + fov * i / definition;
    ARENA_PROPAGATE(cast(environment).transform([&](length_t length) { retval.push_back(length); }).or_else([&](auto) {
      angle = middle_angle;
    }));
  }

  angle = middle_angle;

  return retval;
}
