from arena import *
import pytest

def test_get_component_from_an_entity():
  env = Environment()
  entity = env.create(Bot(x=0.5, y=0.3, mass=1))
  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(0.5)
  assert body.position[1] == pytest.approx(0.3)

def test_create_a_bot_and_drive_it():
  def logic(body: Body):
    body.velocity = (1, 0.5)

  env = Environment()
  entity = env.create(Bot(x=0, y=0, mass=1))
  env.attach(entity, Host(logic))
  for _ in range(10):
    env.step(1 / 10)

  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(1)
  assert body.position[1] == pytest.approx(0.5)

def test_change_position_and_angle_of_body():
  def reposition(body: Body):
    body.set_position((0.1, 0.1))
    body.set_angle(3)

  env = Environment(width=1, height=1)
  id = env.create(Bot(x=0, y=0, mass=1))
  env.attach(id, Host(reposition))

  body = env.get(id, Body)

  assert body.position[0] == pytest.approx(0)
  assert body.position[1] == pytest.approx(0)

  env.step(1)

  assert body.position[0] == pytest.approx(0.1)
  assert body.position[1] == pytest.approx(0.1)
  assert body.angle == pytest.approx(3)

def test_distinguish_collision_parties():
    def forward(body: Body):
        body.velocity = (0.1, 0)

    def backward(body: Body):
        body.velocity = (-0.1, 0)

    env = Environment(width=3, height=3)
    id1 = env.create(Bot(x=-1, y=0, mass=1))
    env.attach(id1, Host(forward))
    id2 = env.create(Bot(x=1, y=0, mass=1))
    env.attach(id2, Host(backward))
    id3 = env.create(Bot(x=0.5, y=0.2, mass=0.1))

    expected_collision = [(id1, id2), (id2, id3)]
    
    def on_collision(e1, e2):
        assert (e1, e2) in expected_collision or (e2, e1) in expected_collision
    
    env.on_collision(on_collision)

    for _ in range(300):
        env.step(1 / 30)


