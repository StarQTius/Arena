#include <arena/2021/cup.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <vector>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <ltl/Range/Value.h>
#include <pybind11/embed.h>
#include <units/isq/si/length.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/environment.hpp>

TEST_CASE("Field interaction with contained bodies", "[.integration]") {
  using namespace arena;
  using namespace units::isq::si::literals;

  Environment environment([](entt::registry &) {});
  create(environment.world, environment.registry, entity::Field{.width = 10_q_m, .height = 10_q_m});

  SECTION("Move around within the bound of the field") {
    using namespace arena::entity;
    using namespace arena::component;
    namespace py = pybind11;

    py::scoped_interpreter guard;

    py::exec(R"(
      def noop():
        pass
    )");

    auto hitbox = make_circle_shape(10_q_cm);
    std::vector<std::tuple<distance_t, distance_t, b2Vec2>> init_parameters{
        {1_q_m, 0_q_m, {1, 0}}, {-1_q_m, 0_q_m, {-1, 0}}, {0_q_m, 1_q_m, {0, 1}},   {0_q_m, -1_q_m, {0, -1}},
        {4_q_m, 4_q_m, {1, 1}}, {4_q_m, -4_q_m, {1, -1}}, {-4_q_m, 4_q_m, {-1, 1}}, {-4_q_m, -4_q_m, {-1, -1}}};

    for (auto &&[x, y, velocity_vector] : init_parameters) {
      auto bot_self = create(environment.world, environment.registry,
                             entity::Bot{.x = x, .y = y, .mass = 1_q_kg, .logic = pybind11::globals()["noop"]}, hitbox);
      environment.registry.emplace<b2Vec2>(bot_self, velocity_vector);
    }

    for ([[maybe_unused]] auto x : ltl::valueRange(0, 100)) {
      for (auto &&[self, body_ptr, velocity] : environment.registry.view<BodyPtr, b2Vec2>().each())
        body_ptr->SetLinearVelocity(velocity);
      environment.world.Step((1._q_s / 20).number(), 8, 3);
    }

    for (auto &&[entity, body_ptr] : environment.registry.view<BodyPtr>().each()) {
      REQUIRE(body_ptr->GetPosition().x >= -5);
      REQUIRE(body_ptr->GetPosition().x <= 5);
      REQUIRE(body_ptr->GetPosition().y >= -5);
      REQUIRE(body_ptr->GetPosition().y <= 5);
    }
  }
}

TEST_CASE("Cup interaction with contained bodies", "[.integration]") {
  using namespace arena;
  using namespace units::isq::si::literals;

  Environment environment{[](entt::registry &) {}};

  SECTION("Cup can be moved on collision") {
    using namespace arena::entity;
    using namespace arena::component;
    namespace py = pybind11;

    py::scoped_interpreter guard;

    py::exec(R"(
      def noop():
        pass
    )");

    auto hitbox = make_circle_shape(10_q_cm);

    auto middle_cup_self =
        create(environment.world, environment.registry, Cup{.x = 10_q_cm, .y = 0_q_cm, .color = Color::RED});
    auto upper_cup_self =
        create(environment.world, environment.registry, Cup{.x = 10_q_cm, .y = 5_q_cm, .color = Color::GREEN});
    auto lower_cup_self =
        create(environment.world, environment.registry, Cup{.x = 10_q_cm, .y = -5_q_cm, .color = Color::GREEN});
    auto bot_self =
        create(environment.world, environment.registry,
               Bot{.x = -30_q_cm, .y = 0_q_cm, .mass = 1_q_kg, .logic = pybind11::globals()["noop"]}, hitbox);

    environment.registry.emplace<b2Vec2>(middle_cup_self, 0.1, 0);
    environment.registry.emplace<b2Vec2>(upper_cup_self, 0.1, 0.05);
    environment.registry.emplace<b2Vec2>(lower_cup_self, 0.1, -0.05);
    environment.registry.emplace<b2Vec2>(bot_self, 0.5, 0);

    for ([[maybe_unused]] auto x : ltl::valueRange(0, 100)) {
      for (auto &&[self, body_ptr, py_host, velocity] : environment.registry.view<BodyPtr, PyHost, b2Vec2>().each())
        body_ptr->SetLinearVelocity(velocity);
      environment.world.Step((1.0_q_s / 20).number(), 8, 3);
    }

    for (auto &&[self, body_ptr, color, initial_position] :
         environment.registry.view<BodyPtr, Color, b2Vec2>().each()) {
      REQUIRE(body_ptr->GetPosition().x != Catch::Approx(initial_position.x));
      REQUIRE(body_ptr->GetPosition().y != Catch::Approx(initial_position.y));
    }
  }
}
