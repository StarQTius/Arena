#include <catch2/catch_test_macros.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

TEST_CASE("Environment renderer from Python", "[.manual]") {
  py::scoped_interpreter guard;

  SECTION("Render a bot alone") {
    py::exec(R"(
      from arena import *

      def move_forward(body: Body):
        body.velocity = (1, 1)

      env = Environment(with_rendering=True)
      env.create(Bot(x=0, y=0, mass=1, logic=move_forward))
      for _ in range(300):
        env.step(0.01)
    )");
  }
}
