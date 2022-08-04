#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <string>
#include <type_traits>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <box2d/b2_types.h>
#include <box2d/b2_world.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <entt/signal/sigh.hpp>
#include <ltl/operator.h>
#include <tl/expected.hpp>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace arena;
using namespace units::isq::si::literals;

namespace {

b2World *world_p = nullptr;
std::function<void(CollisionBeginning &)> collision_callback;

void on_collision_beginning(CollisionBeginning &event) { collision_callback(event); }

struct monitor_t {
  int x;
};

} // namespace

template <> struct arena_component_info<monitor_t> {
  static void init(entt::registry &registry) {
    world_p = &arena::get_world(registry);

    auto &dispatcher = get_dispatcher(registry);
    dispatcher.sink<CollisionBeginning>().connect<&on_collision_beginning>();
  }
};

TEST_CASE("Body component", "[Body][Base]") {

  Environment environment([](auto &&...) {});
  environment.attach(environment.create(), monitor_t{});

  SECTION("Bodies are created upon insertion into registry") {
    expected(world_p, Error{})
        .map([&](b2World &world) {
          auto initial_body_count = world.GetBodyCount();

          auto entity = environment.create();
          environment.attach<b2Body *>(entity, b2BodyDef{});

          REQUIRE(world.GetBodyCount() == initial_body_count + 1);
        })
        .or_else([](auto) { FAIL(); });
  }

  SECTION("Bodies are destroyed upon removal from registry") {
    expected(world_p, Error{})
        .map([&](b2World &world) {
          auto entity = environment.create();
          environment.attach<b2Body *>(entity, b2BodyDef{});

          auto initial_body_count = world.GetBodyCount();

          REQUIRE(environment.remove<b2Body *>(entity) == 1);
          REQUIRE(world.GetBodyCount() == initial_body_count - 1);
        })
        .or_else([](auto) { FAIL(); });
  }
}

TEST_CASE("Collision detection", "[base][collision]") {
  Environment environment{_((...), )};
  environment.attach(environment.create(), monitor_t{});

  SECTION("A event is emitted on collision and a callback can be hooked to that event") {
    struct success : std::exception {};

    b2BodyDef body_def;
    auto circle_shape = component::make_circle_shape(10_q_cm);
    auto e1 = environment.create(), e2 = environment.create();
    body_def.type = b2_dynamicBody;

    body_def.position = {box2d_number(-50_q_cm), box2d_number(0_q_cm)};
    body_def.linearVelocity = {box2d_number(1_q_m_per_s), box2d_number(0_q_m_per_s)};
    auto *b1_p = environment.attach<b2Body *>(e1, body_def);
    b1_p->CreateFixture(&circle_shape, box2d_number(compute_shape_density(circle_shape, 1_q_g)));

    body_def.position = {box2d_number(50_q_cm), box2d_number(0_q_cm)};
    body_def.linearVelocity = {box2d_number(-1_q_m_per_s), box2d_number(0_q_m_per_s)};
    auto *b2_p = environment.attach<b2Body *>(e2, body_def);
    b2_p->CreateFixture(&circle_shape, box2d_number(compute_shape_density(circle_shape, 1_q_g)));

    collision_callback = [&](CollisionBeginning event) {
      REQUIRE_THAT((std::vector{event.entity_a, event.entity_b}),
                   Catch::Matchers::UnorderedEquals(std::vector{e1, e2}));
      throw success{};
    };

    REQUIRE_THROWS_AS(environment.step(1_q_s), success);
  }

  collision_callback = nullptr;
}
