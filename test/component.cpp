#include <arena/component/body.hpp>
#include <arena/component/host.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>
#include <entt/entity/registry.hpp>
#include <ltl/Range/seq.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace Catch::literals;

PYBIND11_EMBEDDED_MODULE(module, m) {
  using namespace arena;
  using namespace arena::component;

  py::class_<b2Body, ObserverPtr<b2Body>>(m, "Body").def_property(
      "velocity",
      [](const b2Body &self) {
        auto &velocity = self.GetLinearVelocity();
        return py::make_tuple(velocity.x, velocity.y);
      },
      [](b2Body &self, py::tuple value) {
        self.SetLinearVelocity({py::cast<float>(value[0]), py::cast<float>(value[1])});
      });
}

TEST_CASE("Single host interactions with environment", "[.integration]") {
  b2World world{{.0f, .0f}};
  entt::registry registry;

  SECTION("Entities with PyHost and BodyPtr components collide with other shapes") {
    using namespace arena;
    using namespace arena::component;

    py::scoped_interpreter guard;

    FetcherMap fetchers{{"Body", get_component<BodyPtr>}};
    py::exec(R"(
      from module import Body

      def go_left(body: Body):
        body.velocity = (1, 0)
    )");

    // PyHost setup
    b2BodyDef pyhost_body_def;
    b2PolygonShape pyhost_hitbox;
    pyhost_body_def.type = b2_dynamicBody;
    pyhost_body_def.position = {0.f, 0.f};
    pyhost_hitbox.SetAsBox(1.f, 1.f);
    auto *pyhost_body_ptr = world.CreateBody(&pyhost_body_def);
    pyhost_body_ptr->CreateFixture(&pyhost_hitbox, 1.f);
    auto pyhost_self = registry.create();
    registry.emplace<BodyPtr>(pyhost_self, pyhost_body_ptr);
    registry.emplace<PyHost>(pyhost_self, py::globals()[py::str{"go_left"}]);

    // Obstacle setup
    b2BodyDef obstacle_body_def;
    b2PolygonShape obstacle_hitbox;
    obstacle_body_def.position = {5.f, 0.f};
    obstacle_hitbox.SetAsBox(1.f, 10.f);
    auto *obstacle_body_ptr = world.CreateBody(&obstacle_body_def);
    obstacle_body_ptr->CreateFixture(&obstacle_hitbox, 0.f);
    auto obstacle_self = registry.create();
    registry.emplace<BodyPtr>(obstacle_self, obstacle_body_ptr);

    registry.view<PyHost>().each([&](auto self, auto &pyhost) { pyhost.invoke(registry, self, fetchers); });

    for ([[maybe_unused]] auto x : ltl::valueRange(0, 10))
      world.Step(1.f, 8, 3);

    REQUIRE(registry.get<BodyPtr>(pyhost_self)->GetPosition().x == (3._a).margin(.1f));
    REQUIRE(registry.get<BodyPtr>(pyhost_self)->GetPosition().y == (0._a).margin(.1f));
  }
}
