#include <arena/component/host.hpp>
#include <arena/environment.hpp>

#include <catch2/catch_test_macros.hpp>

#include <pybind11/cast.h>
#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;

using namespace arena;
using namespace arena::component;

TEST_CASE("PyHost class testing", "[PyHost][Base]") {
  Environment environment{[](auto &) {}};
  auto self = environment.registry.create();

  py::exec(R"(
    _globals = globals()
    _globals_backup = dict(_globals)
  )");

  SECTION("PyHost callback is called on the entity components") {
    FetcherMap fetchers{{"int", get_component<int>}, {"float", get_component<float>}};

    py::exec(R"(
      value = 0
      def function(a : int, b : float):
        global value
        value = a + b)");

    environment.registry.emplace<PyHost>(self, py::globals()[py::str{"function"}]);
    environment.registry.emplace<int>(self, 8);
    environment.registry.emplace<float>(self, 16);

    environment.registry.view<PyHost>().each(
        [&](auto self, auto &pyhost) { pyhost.invoke(environment, self, fetchers); });

    REQUIRE(py::int_{py::globals()["value"]} == 8 + 16);
  }

  py::exec(R"(
    _globals = dict(_globals_backup)
  )");
}
