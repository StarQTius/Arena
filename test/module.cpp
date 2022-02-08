#include <memory>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <arena/component/body.hpp>
#include <arena/component/host.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;
using namespace arena;
using namespace Catch::literals;
using namespace py::literals;

TEST_CASE("C++/Python binding", "[.integration]") {
  py::scoped_interpreter guard;

  SECTION("Create a bot and drive it") {
    py::exec(R"(
      from arena import *

      def logic(body: Body):
        body.velocity = (0.5, 1)

      env = Environment()
      env.create(Bot(x=0, y=0, mass=1, logic=logic))
      for _ in range(30):
        env.step(1 / 10)
    )");

    auto &environment = py::globals()["env"].cast<Environment &>();
    for (auto &&[self, body_ptr, py_host] : environment.registry.view<component::BodyPtr, component::PyHost>().each()) {
      REQUIRE(body_ptr->GetPosition().x == 1.5_a);
      REQUIRE(body_ptr->GetPosition().y == 3_a);
    }
  }
}
