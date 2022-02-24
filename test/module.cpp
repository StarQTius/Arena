#include <iterator>
#include <memory>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <arena/2021/cup.hpp>
#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;
using namespace arena;
using namespace Catch::literals;
using namespace py::literals;

TEST_CASE("C++/Python binding", "[.integration][binding]") {
  py::scoped_interpreter guard;

  SECTION("Create a bot and drive it") {
    py::exec(R"(
      from arena import *

      def logic(body: Body):
        body.velocity = (1, 0.5)

      env = Environment()
      env.create(Bot(x=0, y=0, mass=1, logic=logic, cup_capacity=0))
      for _ in range(10):
        env.step(1 / 10)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();
    for (auto &&[self, body_ptr, py_host] : environment.registry.view<component::BodyPtr, component::PyHost>().each()) {
      REQUIRE(body_ptr->GetPosition().x == 1_a);
      REQUIRE(body_ptr->GetPosition().y == 0.5_a);
    }
  }

  SECTION("Create a cup and move it with a bot") {
    py::exec(R"(
      from arena import *

      def move_forward(body: Body):
        body.velocity = (1, 0)

      env = Environment()
      env.create(Bot(x=-1.5, y=0, mass=1, logic=move_forward, cup_capacity=0))
      env.create(Cup(x=1, y=0, color=CupColor.RED))
      for _ in range(500):
        env.step(0.1)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();
    for (auto &&[self, body_ptr, color] : environment.registry.view<component::BodyPtr, component::CupColor>().each()) {
      REQUIRE(body_ptr->GetPosition().x > 1);
      REQUIRE(body_ptr->GetPosition().y == 0_a);
    }
  }

  SECTION("Let a bot grab a cup in range by filtering 'Environment.cups' with 'filter'") {
    REQUIRE_THROWS(py::exec(R"(
      from arena import *

      def distance_squared(body_a, body_b):
        return (body_a.position[0] - body_b.position[0]) ** 2 + (body_a.position[1] - body_b.position[1]) ** 2

      def grab(env: Environment, body: Body, cup_grabber: CupGrabber):
        inrange_cups = filter(lambda x: distance_squared(body, x[1]) < 4, env.cups)
        cup_grabber.grab(next(inrange_cups)[0])

      env = Environment()
      env.create(Bot(x=-1.5, y=0, mass=1, logic=grab, cup_capacity=2))
      env.create(Cup(x=0, y=0, color=CupColor.RED))
      env.create(Cup(x=1, y=0, color=CupColor.GREEN))
      env.step(1)
      env.step(1)
    )"));

    auto &environment = py::globals()["env"].cast<Environment &>();
    auto view = environment.registry.view<component::CupColor>();

    REQUIRE(environment.world.GetBodyCount() == 3);
    REQUIRE(view.get<component::CupColor>(*view.begin()) == component::CupColor::GREEN);
  }

  SECTION("Let a bot grab a cup then drop it") {
    py::exec(R"(
      from arena import *

      flag = False
      def grab_n_drop(env: Environment, body: Body, cup_grabber: CupGrabber):
        global flag
        if not flag:
          cup_grabber.grab(next(env.cups)[0])
        else:
          cup_grabber.drop(Cup(x=0.5, y=0, color=CupColor.RED))
        flag = True

      env = Environment()
      env.create(Bot(x=-1, y=0, mass=1, logic=grab_n_drop, cup_capacity=2))
      env.create(Cup(x=0, y=0, color=CupColor.RED))
      env.step(1)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();

    REQUIRE(environment.world.GetBodyCount() == 2);

    py::exec(R"(env.step(1))");
    auto view = environment.registry.view<component::BodyPtr, component::CupColor>();

    REQUIRE(environment.world.GetBodyCount() == 3);
    REQUIRE(view.get<component::CupColor>(*view.begin()) == component::CupColor::RED);
    REQUIRE(view.get<component::BodyPtr>(*view.begin())->GetPosition().x == 0.5_a);
    REQUIRE(view.get<component::BodyPtr>(*view.begin())->GetPosition().y == 0_a);
  }

  SECTION("Access a cup grabber storage") {
    py::exec(R"(
      from arena import *

      red_count = 0
      green_count = 0
      def grab_all(env: Environment, cup_grabber: CupGrabber):
        global red_count, green_count
        for entity, _0, _1 in env.cups:
          cup_grabber.grab(entity)
        red_count = cup_grabber.storage[CupColor.RED]
        green_count = cup_grabber.storage[CupColor.GREEN]

      env = Environment()
      env.create(Bot(x=-1, y=0, mass=1, logic=grab_all, cup_capacity=5))
      env.create(Cup(x=0, y=0, color=CupColor.RED))
      env.create(Cup(x=0.2, y=0, color=CupColor.GREEN))
      env.create(Cup(x=0.4, y=0, color=CupColor.RED))
      env.create(Cup(x=0.6, y=0, color=CupColor.GREEN))
      env.create(Cup(x=0.8, y=0, color=CupColor.RED))
      env.step(1)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();

    REQUIRE(environment.world.GetBodyCount() == 2);
    REQUIRE(py::globals()["red_count"].cast<size_t>() == 3);
    REQUIRE(py::globals()["green_count"].cast<size_t>() == 2);
  }

  SECTION("Create a field of custom dimension") {
    py::exec(R"(
      from arena import *

      def go_forward(body: Body):
        body.forward_velocity = 1

      env = Environment(width=1, height=1)
      env.create(Bot(x=-1, y=0, mass=1, logic=go_forward, cup_capacity=0))
      for _ in range(5):
        env.step(1)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();
    auto &bot = *environment.registry.view<component::PyHost>().begin();

    REQUIRE(environment.registry.get<component::BodyPtr>(bot)->GetPosition().x < 0.5);
    REQUIRE(environment.registry.get<component::BodyPtr>(bot)->GetPosition().x > -0.5);
    REQUIRE(environment.registry.get<component::BodyPtr>(bot)->GetPosition().y < 0.5);
    REQUIRE(environment.registry.get<component::BodyPtr>(bot)->GetPosition().y > -0.5);
  }
}
