#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

#include <box2d/b2_circle_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/registry.hpp>
#include <ltl/operator.h>
#include <units/generic/angle.h>
#include <units/isq/si/area.h>
#include <units/isq/si/length.h>
#include <units/isq/si/mass.h>
#include <units/isq/si/speed.h>
#include <units/isq/si/time.h>
#include <units/math.h>
#include <units/quantity_io.h>

#include <arena/component/body.hpp>
#include <arena/component/ray.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>
#include <arena/system/raycasting.hpp>

using namespace arena;

TEST_CASE("Ray component testing", "[base][ray][component]") {
  using namespace units::isq::si::literals;
  using namespace units::literals;

  Environment environment{_((...), )};

  b2BodyDef body_def;
  auto circle_shape = component::make_circle_shape(10_q_cm);
  body_def.type = b2_dynamicBody;

  auto entity = environment.create();
  body_def.position = {box2d_number(0_q_m), box2d_number(0_q_m)};
  auto *body_p = environment.attach<b2Body *>(entity, body_def);
  body_p->CreateFixture(&circle_shape, box2d_number(1_q_kg / 1_q_m2));
  auto &ray = environment.attach(entity, component::Ray{.x = 0_q_m, .y = 0_q_m, .range = 10_q_m, .angle = 0_q_rad});

  SECTION("Ray are anchored to an entity and measure the distance to the neareast body") {
    REQUIRE(abs(system::cast(ray, body_p) - 10_q_m) < units::epsilon<length_t>());

    body_def.position = {box2d_number(5_q_m), box2d_number(0_q_m)};
    environment.attach<b2Body *>(environment.create(), body_def)
        ->CreateFixture(&circle_shape, box2d_number(1_q_kg / 1_q_m2));
    REQUIRE(abs(system::cast(ray, body_p) - (5_q_m - 10_q_cm)) < units::epsilon<length_t>());

    body_p->SetLinearVelocity({box2d_number(1_q_m_per_s), box2d_number(0_q_m_per_s)});
    environment.step(1_q_s);

    REQUIRE(abs(system::cast(ray, body_p) - (4_q_m - 10_q_cm)) < units::epsilon<length_t>());

    body_p->SetLinearVelocity({box2d_number(0_q_m_per_s), box2d_number(0_q_m_per_s)});
    body_p->SetAngularVelocity(box2d_number(M_PI * 1_q_rad / 1_q_s));
    environment.step(1_q_s);

    REQUIRE(abs(system::cast(ray, body_p) - 10_q_m) < units::epsilon<length_t>());
  }

  SECTION("Ray components can sweep around their main axis") {
    body_def.position = {box2d_number(5_q_m), box2d_number(0_q_m)};
    environment.attach<b2Body *>(environment.create(), body_def)
        ->CreateFixture(&circle_shape, box2d_number(1_q_kg / 1_q_m2));
    body_def.position = {box2d_number(-5_q_m), box2d_number(0_q_m)};
    environment.attach<b2Body *>(environment.create(), body_def)
        ->CreateFixture(&circle_shape, box2d_number(1_q_kg / 1_q_m2));

    auto lengths = system::sweep(ray, body_p, pi, 10);
    REQUIRE(abs(lengths.front() - (5_q_m - 10_q_cm)) < units::epsilon<length_t>());
    REQUIRE(abs(lengths.back() - (5_q_m - 10_q_cm)) < units::epsilon<length_t>());
  }
}
