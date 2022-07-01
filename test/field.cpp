#include <catch2/catch_test_macros.hpp>

#include <array>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <box2d/b2_body.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <ltl/Range/Value.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/time.h>

#include <arena/2021/cup.hpp>
#include <arena/component/body.hpp>
#include <arena/concept.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace py = pybind11;

TEST_CASE("Field interaction with contained bodies", "[Field][Base]") {
  using namespace arena;
  using namespace units::isq::si::literals;

  Environment environment([](Environment &) {});
  create(environment.world, environment.registry, entity::Field{.width = 10_q_m, .height = 10_q_m});

  py::exec(R"(
    _globals = globals()
    _globals_backup = dict(_globals)
  )");

  SECTION("Move around within the bound of the field") {
    using namespace arena::entity;
    using namespace arena::component;
    namespace py = pybind11;

    py::exec(R"(
      def noop():
        pass
    )");

    std::vector<std::tuple<length_t, length_t, b2Vec2>> init_parameters{
        {1_q_m, 0_q_m, {1, 0}}, {-1_q_m, 0_q_m, {-1, 0}}, {0_q_m, 1_q_m, {0, 1}},   {0_q_m, -1_q_m, {0, -1}},
        {4_q_m, 4_q_m, {1, 1}}, {4_q_m, -4_q_m, {1, -1}}, {-4_q_m, 4_q_m, {-1, 1}}, {-4_q_m, -4_q_m, {-1, -1}}};

    for (auto &&[x, y, velocity_vector] : init_parameters) {
      auto bot_self = create(
          environment.world, environment.registry,
          entity::Bot{.x = x, .y = y, .mass = 1_q_kg, .logic = pybind11::globals()["noop"], .cup_storage_size = 0});
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

  py::exec(R"(
    _globals = dict(_globals_backup)
  )");
}
