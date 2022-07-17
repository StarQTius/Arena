#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

#include <box2d/b2_types.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <pybind11/pybind11.h>

#include <arena/component/body.hpp>
#include <arena/environment.hpp>

namespace py = pybind11;

TEST_CASE("Body component", "[Body][Base]") {
  using namespace arena;

  Environment environment([](auto &&...) {});

  SECTION("Bodies are created upon insertion into registry") {
    auto initial_body_count = environment.world->GetBodyCount();
    b2BodyDef def;

    auto entity = environment.registry.create();
    environment.attach(entity, environment.world->CreateBody(&def));

    REQUIRE(environment.world->GetBodyCount() == initial_body_count + 1);
  }

  SECTION("Bodies are destroyed upon removal from registry") {
    b2BodyDef def;

    auto entity = environment.registry.create();
    environment.attach(entity, environment.world->CreateBody(&def));

    auto initial_body_count = environment.world->GetBodyCount();

    environment.registry.remove<b2Body *>(entity);

    REQUIRE(environment.world->GetBodyCount() == initial_body_count - 1);
  }
}
