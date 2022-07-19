#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

#include <box2d/b2_types.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <pybind11/pybind11.h>
#include <tl/expected.hpp>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;

namespace {

struct monitor_t {
  int x;
};

constexpr auto arena_component_info(monitor_t *) {
  struct {
  } info;
  return info;
}

b2World *world_p = nullptr;

} // namespace

namespace arena {

template <> struct init_guard<monitor_t> {
  static void init(entt::registry &registry) { world_p = &arena::get_world(registry); }
};

} // namespace arena

TEST_CASE("Body component", "[Body][Base]") {
  using namespace arena;

  Environment environment([](auto &&...) {});
  environment.attach(environment.create(), monitor_t{});

  SECTION("Bodies are created upon insertion into registry") {
    expected(world_p, Error{})
        .map([&](b2World &world) {
          auto initial_body_count = world.GetBodyCount();

          auto entity = environment.create();
          environment.attach(entity, b2BodyDef{});

          REQUIRE(world.GetBodyCount() == initial_body_count + 1);
        })
        .or_else([](auto) { FAIL(); });
  }

  SECTION("Bodies are destroyed upon removal from registry") {
    expected(world_p, Error{})
        .map([&](b2World &world) {
          auto entity = environment.create();
          environment.attach(entity, b2BodyDef{});

          auto initial_body_count = world.GetBodyCount();

          REQUIRE(environment.remove<b2Body *>(entity) == 1);
          REQUIRE(world.GetBodyCount() == initial_body_count - 1);
        })
        .or_else([](auto) { FAIL(); });
  }
}
