from arena import *
import pytest

def test_get_component_from_an_entity():
  env = Environment()
  entity = env.create(Bot(x=500, y=300, mass=1))
  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(500)
  assert body.position[1] == pytest.approx(300)

def test_create_a_bot_and_drive_it():
  def logic(body: Body):
    body.velocity = (1000, 500)

  env = Environment()
  entity = env.create(Bot(x=0, y=0, mass=1))
  env.attach(entity, Host(logic))
  for _ in range(10):
    env.step(1 / 10)

  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(1000)
  assert body.position[1] == pytest.approx(500)

def test_change_position_and_angle_of_body():
  def reposition(body: Body):
    body.set_position((100, 100))
    body.set_angle(3)

  env = Environment(width=1000, height=1000)
  id = env.create(Bot(x=0, y=0, mass=1))
  env.attach(id, Host(reposition))

  body = env.get(id, Body)

  assert body.position[0] == pytest.approx(0)
  assert body.position[1] == pytest.approx(0)

  env.step(1)

  assert body.position[0] == pytest.approx(100)
  assert body.position[1] == pytest.approx(100)
  assert body.angle == pytest.approx(3)

def test_distinguish_collision_parties():
    def forward(body: Body):
        body.velocity = (100, 0)

    def backward(body: Body):
        body.velocity = (-100, 0)

    env = Environment(width=3000, height=3000)
    id1 = env.create(Bot(x=-1000, y=0, mass=1))
    env.attach(id1, Host(forward))
    id2 = env.create(Bot(x=1000, y=0, mass=1))
    env.attach(id2, Host(backward))
    id3 = env.create(Bot(x=500, y=200, mass=0.1))

    expected_collision = [(id1, id2), (id2, id3)]
    
    def on_collision(e1, e2):
        assert (e1, e2) in expected_collision or (e2, e1) in expected_collision
    
    env.on_collision(on_collision)

    for _ in range(300):
        env.step(1 / 30)

def test_hook_function_to_ray_cast_signal():
    result = ()
    def ray_cast_hook(entity, x, y, angle, distance):
        nonlocal result
        result = (x, y, angle) 

    env = Environment()
    entity = env.create(Bot(x=1000, y=500, mass=1))
    env.attach(entity, Ray(x=0, y=0, angle=0, range=1000))
    env.on_ray_fired(ray_cast_hook)

    body = env.get(entity, Body)
    body.set_angle(1)
    env.get(entity, Ray).cast()

    assert result == (1000, 500, 1)

def test_filter_with_ray_cast():
    env = Environment()
    entity = env.create(Bot(x=-1000, y=0, mass=1))
    env.attach(entity, Ray(range=3000, filter=lambda e: env.all_of(e, [Ray])))

    e1, e2 = env.create(Bot(x=0, y=0, mass=1)), env.create(Bot(x=1000, y=0, mass=1))
    env.attach(e2, Ray(range=0))

    assert env.get(entity, Ray).cast() > 1000
