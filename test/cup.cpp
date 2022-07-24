#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <type_traits>
#include <utility>

#include <box2d/b2_math.h>
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
#include <arena/component/host.hpp>
#include <arena/entity/bot.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

namespace py = pybind11;

namespace {

struct monitor_t {
  arena::length_t x, y;
};

constexpr auto arena_component_info(monitor_t *) {
  struct {
  } info;
  return info;
}

} // namespace

TEST_CASE("Cup interaction with contained bodies", "[Cup][STW]") {
  using arena::Environment;
  using namespace units::isq::si::literals;

  Environment environment{[](auto &&...) {}};

  py::exec(R"(
    _globals = globals()
    _globals_backup = dict(_globals)
  )");

  SECTION("Cup can be moved on collision") {
    using namespace arena;
    using namespace arena::entity;
    using namespace arena::component;
    using namespace arena::stw::entity;
    using namespace arena::stw::component;
    namespace py = pybind11;

    py::exec(R"(
      def noop():
        pass
    )");

    auto hitbox = make_circle_shape(10_q_cm);

    auto middle_cup_self = environment.create(Cup{.x = 10_q_cm, .y = 0_q_cm, .color = CupColor::RED});
    auto upper_cup_self = environment.create(Cup{.x = 10_q_cm, .y = 5_q_cm, .color = CupColor::GREEN});
    auto lower_cup_self = environment.create(Cup{.x = 10_q_cm, .y = -5_q_cm, .color = CupColor::GREEN});
    auto bot_self =
        environment.create(Bot{.x = -30_q_cm, .y = 0_q_cm, .mass = 1_q_kg, .logic = pybind11::globals()["noop"]});

    environment.attach(middle_cup_self, monitor_t{0.1_q_m, 0_q_m});
    environment.attach(upper_cup_self, monitor_t{0.1_q_m, 0.05_q_m});
    environment.attach(lower_cup_self, monitor_t{0.1_q_m, -0.05_q_m});
    environment.attach(bot_self, monitor_t{0.5_q_m, 0_q_m});

    for ([[maybe_unused]] auto x : ltl::valueRange(0, 100)) {
      for (auto &&[self, body_p, py_host, vec] : environment.view<b2Body *, PyHost, monitor_t>().each())
        body_p->SetLinearVelocity({box2d_number(vec.x / 1.0_q_s), box2d_number(vec.y / 1.0_q_s)});
      environment.step(1.0_q_s / 20);
    }

    for (auto &&[self, body_p, color, initial_position] : environment.view<b2Body *, CupColor, monitor_t>().each()) {
      REQUIRE(body_p->GetPosition().x != Catch::Approx(box2d_number(initial_position.x)));
      REQUIRE(body_p->GetPosition().y != Catch::Approx(box2d_number(initial_position.y)));
    }
  }

  py::exec(R"(
    _globals = dict(_globals_backup)
  )");
}
