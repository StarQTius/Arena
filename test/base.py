from arena import *
import pytest
import numpy as np


class MockLogic:
    async def method(self, body: Body):
        while True:
            await wait_next()
    
    @classmethod
    async def class_method(self, body: Body):
        while True:
            await wait_next()
    
    async def bad_method(self, body):
        while True:
            await wait_next()
    
    @classmethod
    async def bad_class_method(self, body):
        while True:
            await wait_next()

def test_get_component_from_an_entity():
  env = Environment()
  entity = env.create(Bot(x=500, y=300, mass=1))
  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(500)
  assert body.position[1] == pytest.approx(300)

def test_create_a_bot_and_drive_it():
  async def logic(body: Body):
    while True:
        body.velocity = (1000, 500)
        await wait_next()

  env = Environment()
  entity = env.create(Bot(x=0, y=0, mass=1))
  env.attach(entity, Host(logic))
  for _ in range(10):
    env.step(1 / 10)

  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(1000)
  assert body.position[1] == pytest.approx(500)

def test_change_position_and_angle_of_body():
  async def reposition(body: Body):
      while True:
          body.set_position((100, 100))
          body.set_angle(3)
          await wait_next()

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
    async def forward(body: Body):
        while True:
            body.velocity = (100, 0)
            await wait_next()

    async def backward(body: Body):
        while True:
            body.velocity = (-100, 0)
            await wait_next()

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

def test_custom_caster():
    with pytest.raises(RuntimeError):
        Environment(width=(0, 0), height=500)

    with pytest.raises(TypeError):
        env = Environment()
        e = env.create(Bot(x=0, y=0, mass=1))
        env.get(e, Body).set_position(64)

def test_logic_method():
    env = Environment()
    e1, e2 = env.create(Bot(x=0, y=0, mass=1)), env.create(Bot(x=0, y=0, mass=1))
    env.attach(e1, Host(MockLogic().method))
    env.attach(e2, Host(MockLogic.class_method))
    
    env.step(0)

    with pytest.raises(RuntimeError):
        bad_e = env.create(Bot(x=0, y=0, mass=1))
        env.attach(bad_e, Host(MockLogic().bad_method))
        env.step(0)

    with pytest.raises(RuntimeError):
        bad_e = env.create(Bot(x=0, y=0, mass=1))
        env.attach(bad_e, Host(MockLogic.bad_class_method))
        env.step(0)
