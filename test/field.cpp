#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <box2d/b2_math.h>
#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>
#include <ltl/Range/Value.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>

#include <arena/component/body.hpp>
#include <arena/entity/bot.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace py = pybind11;

namespace {

struct monitor_t {
  arena::speed_t x, y;
};

} // namespace

template <> struct arena_component_info<monitor_t> {};

TEST_CASE("Field interaction with contained bodies", "[Field][Base]") {
  using namespace arena;
  using namespace units::isq::si::literals;

  Environment environment([](auto &&...) {});
  environment.create(entity::Field{.width = 10_q_m, .height = 10_q_m});

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

    std::vector<ltl::tuple_t<length_t, length_t, monitor_t>> init_parameters{
        {1_q_m, 0_q_m, {1_q_m_per_s, 0_q_m_per_s}},   {-1_q_m, 0_q_m, {-1_q_m_per_s, 0_q_m_per_s}},
        {0_q_m, 1_q_m, {0_q_m_per_s, 1_q_m_per_s}},   {0_q_m, -1_q_m, {0_q_m_per_s, -1_q_m_per_s}},
        {4_q_m, 4_q_m, {1_q_m_per_s, 1_q_m_per_s}},   {4_q_m, -4_q_m, {1_q_m_per_s, -1_q_m_per_s}},
        {-4_q_m, 4_q_m, {-1_q_m_per_s, 1_q_m_per_s}}, {-4_q_m, -4_q_m, {-1_q_m_per_s, -1_q_m_per_s}}};

    for (auto &&[x, y, velocity_vector] : init_parameters) {
      auto bot_self =
          environment.create(entity::Bot{.x = x, .y = y, .mass = 1_q_kg, .logic = pybind11::globals()["noop"]});
      environment.attach(bot_self, velocity_vector);
    }

    for ([[maybe_unused]] auto x : ltl::valueRange(0, 100)) {
      for (auto &&[self, body_p, velocity] : environment.view<b2Body *, monitor_t>().each())
        body_p->SetLinearVelocity({box2d_number(velocity.x), box2d_number(velocity.y)});
      environment.step(1._q_s / 20);
    }

    for (auto &&[entity, body_p] : environment.view<b2Body *>().each()) {
      REQUIRE(body_p->GetPosition().x >= -5);
      REQUIRE(body_p->GetPosition().x <= 5);
      REQUIRE(body_p->GetPosition().y >= -5);
      REQUIRE(body_p->GetPosition().y <= 5);
    }
  }

  py::exec(R"(
    _globals = dict(_globals_backup)
  )");
}
