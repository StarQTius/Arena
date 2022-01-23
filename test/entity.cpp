#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <vector>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_world.h>
#include <ltl/Range/Value.h>
#include <pybind11/embed.h>
#include <units/isq/si/length.h>
#include <units/isq/si/time.h>

#include <arena/component/physics.hpp>
#include <arena/environment.hpp>

TEST_CASE("Field interaction with contained bodies", "[.integration]") {
  using namespace arena;
  using namespace units::isq::si::literals;

  Environment environment;
  create(environment.world, environment.registry, entity::Field{.width = 10_q_m, .height = 10_q_m});

  SECTION("Move around within the bound of the field") {
    using namespace arena::entity;
    using namespace arena::component;
    namespace py = pybind11;

    py::scoped_interpreter guard;

    auto hitbox = make_circle_shape(10_q_cm);
    std::vector<std::tuple<distance_t, distance_t, b2Vec2>> init_parameters{
        {1_q_m, 0_q_m, {1, 0}}, {-1_q_m, 0_q_m, {-1, 0}}, {0_q_m, 1_q_m, {0, 1}},   {0_q_m, -1_q_m, {0, -1}},
        {4_q_m, 4_q_m, {1, 1}}, {4_q_m, -4_q_m, {1, -1}}, {-4_q_m, 4_q_m, {-1, 1}}, {-4_q_m, -4_q_m, {-1, -1}}};
    py::exec(R"(
      def noop():
        pass
    )");

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
