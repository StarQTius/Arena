#include <type_traits>

#include <box2d/b2_chain_shape.h>
#include <box2d/b2_math.h>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <arena/component/body.hpp>
#include <arena/entity/field.hpp>
#include <arena/environment.hpp>
#include <arena/physics.hpp>

using namespace arena;

entt::entity arena::entity::create(Environment &environment, const arena::entity::Field &def) {
  using namespace units::isq;

  auto entity = environment.create();

  b2BodyDef body_def;
  body_def.position = {0, 0};

  b2ChainShape boundaries;
  auto width = box2d_number(def.width);
  auto height = box2d_number(def.height);
  b2Vec2 boundaries_chains[] = {
      {width / 2, height / 2}, {width / 2, -height / 2}, {-width / 2, -height / 2}, {-width / 2, height / 2}};
  boundaries.CreateLoop(boundaries_chains, 4);

  auto *body_p = environment.attach<b2Body *>(entity, body_def);
  body_p->CreateFixture(&boundaries, 0);

  return entity;
}
