#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/b2_world.h>
#include <box2d/b2_world_callbacks.h>

#include <arena/physics.hpp>
#include <arena/system/raycasting.hpp>
#include <arena/component/body.hpp>
#include <arena/component/ray.hpp>

using namespace arena;

length_t arena::system::cast(const component::Ray &ray, b2Body *body_p) {
  struct : b2RayCastCallback {
    length_t hitpoint_distance;

    virtual float ReportFixture(b2Fixture *, const b2Vec2 &, const b2Vec2 &, float fraction) {
      hitpoint_distance *= fraction;
      return fraction;
    }
  } callback;

  callback.hitpoint_distance = ray.range;

  auto &world = *body_p->GetWorld();
  auto origin = b2Vec2{box2d_number(ray.x), box2d_number(ray.y)} + body_p->GetPosition();

  world.RayCast(
    &callback,
    origin,
    origin + box2d_number(ray.range) * b2Vec2{
    std::cos(box2d_number(ray.angle) + body_p->GetAngle()),
    std::sin(box2d_number(ray.angle) + body_p->GetAngle())});

  return callback.hitpoint_distance;
}
