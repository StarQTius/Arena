#include <component/host.hpp>

#include <map>

#include <catch2/catch_test_macros.hpp>

#include <pybind11/cast.h>
#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;

TEST_CASE("PyHost class testing", "[PyHost]") {
  py::scoped_interpreter guard{};

  entt::registry world;
  auto self = world.create();

  SECTION("PyHost callback is called on the entity components") {
    using component::PyHost;

    py::exec(R"(
      value = 0
      def function(a : int, b : float):
        global value
        value = a + b)");

    FetcherMap fetchers{{"int", get_component<int>}, {"float", get_component<float>}};

    world.emplace<PyHost>(self, py::globals()[py::str{"function"}]);
    world.emplace<int>(self, 8);
    world.emplace<float>(self, 16);

    world.view<PyHost>().each([&](auto self, auto &pyhost) { pyhost.invoke(world, self, fetchers); });

    REQUIRE(py::int_{py::globals()["value"]} == 8 + 16);
  }
}
