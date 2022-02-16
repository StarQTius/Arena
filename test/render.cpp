#include <catch2/catch_test_macros.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <arena/environment.hpp>

namespace py = pybind11;

TEST_CASE("Environment renderer from Python", "[.manual]") {
  py::scoped_interpreter guard;

  SECTION("Render a bot alone") {
    py::exec(R"(
      from arena import *

      def move_forward(body: Body):
        body.velocity = (1, 1)

      env = Environment()
      env.create(Bot(x=0, y=0, mass=1, logic=move_forward, cup_capacity=0))
      with env.renderer:
        for _ in range(300):
          env.step(0.01)
    )");
  }

  SECTION("Render a bot and some cups") {
    py::exec(R"(
      from arena import *

      def move_forward(body: Body):
        body.velocity = (0.5, 0)

      env = Environment()
      env.create(Bot(x=-1, y=0, mass=1, logic=move_forward, cup_capacity=0))
      env.create(Cup(x=0, y=0.07, color=Color.RED))
      env.create(Cup(x=0, y=0, color=Color.GREEN))
      env.create(Cup(x=0, y=-0.07, color=Color.RED))
      with env.renderer:
        for _ in range(300):
          env.step(0.01)
    )");
  }

  SECTION("Render a bot grabbing a cup") {
    py::exec(R"(
      from arena import *

      i = 2
      def grab(env: Environment, body: Body, cup_grabber: CupGrabber):
        global i
        i -= 1
        if i == 0:
          cup_grabber.grab(next(env.cups)[0])

      env = Environment()
      env.create(Bot(x=-1, y=0, mass=1, logic=grab, cup_capacity=2))
      env.create(Cup(x=0, y=0, color=Color.RED))
      with env.renderer:
        env.step(1)
        env.step(1)
        env.step(1)
    )");
  }
}
