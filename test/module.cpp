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
      env.create(Bot(x=0, y=0, mass=1, logic=logic))
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
      env.create(Bot(x=-1.5, y=0, mass=1, logic=move_forward))
      env.create(Cup(x=1, y=0, color=Color.RED))
      for _ in range(500):
        env.step(0.1)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();
    for (auto &&[self, body_ptr, color] : environment.registry.view<component::BodyPtr, component::Color>().each()) {
      REQUIRE(body_ptr->GetPosition().x > 1);
      REQUIRE(body_ptr->GetPosition().y == 0_a);
    }
  }
}
