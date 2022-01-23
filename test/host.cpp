#include <arena/component/host.hpp>

#include <catch2/catch_test_macros.hpp>

#include <pybind11/cast.h>
#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;

TEST_CASE("PyHost class testing", "[component][PyHost]") {
  py::scoped_interpreter guard;

  entt::registry registry;
  auto self = registry.create();

  SECTION("PyHost callback is called on the entity components") {
    using namespace arena;
    using namespace arena::component;

    py::exec(R"(
      value = 0
      def function(a : int, b : float):
        global value
        value = a + b)");

    FetcherMap fetchers{{"int", get_component<int>}, {"float", get_component<float>}};

    registry.emplace<PyHost>(self, py::globals()[py::str{"function"}]);
    registry.emplace<int>(self, 8);
    registry.emplace<float>(self, 16);

    registry.view<PyHost>().each([&](auto self, auto &pyhost) { pyhost.invoke(registry, self, fetchers); });

    REQUIRE(py::int_{py::globals()["value"]} == 8 + 16);
  }
}
