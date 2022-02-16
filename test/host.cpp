#include <arena/component/host.hpp>
#include <arena/environment.hpp>

#include <catch2/catch_test_macros.hpp>

#include <pybind11/cast.h>
#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;

using namespace arena;
using namespace arena::component;

TEST_CASE("PyHost class testing", "[component][PyHost]") {
  py::scoped_interpreter guard;

  Environment environment{[](auto &) {}};
  auto self = environment.registry.create();

  SECTION("PyHost callback is called on the entity components") {
    py::exec(R"(
      value = 0
      def function(a : int, b : float):
        global value
        value = a + b)");

    FetcherMap fetchers{{"int", get_component<int>}, {"float", get_component<float>}};

    environment.registry.emplace<PyHost>(self, py::globals()[py::str{"function"}]);
    environment.registry.emplace<int>(self, 8);
    environment.registry.emplace<float>(self, 16);

    environment.registry.view<PyHost>().each(
        [&](auto self, auto &pyhost) { pyhost.invoke(environment, self, fetchers); });

    REQUIRE(py::int_{py::globals()["value"]} == 8 + 16);
  }
}
