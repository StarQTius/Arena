from arena import *
import pytest

def test_get_component_from_an_entity():
  def noop():
    pass

  env = Environment()
  entity = env.create(Bot(x=0.5, y=0.3, mass=1, logic=noop, cup_capacity=0))
  body = env.get(entity, Body)

  assert body.position[0] == pytest.approx(0.5)
  assert body.position[1] == pytest.approx(0.3)

def test_create_a_bot_and_drive_it():
  def logic(body: Body):
    body.velocity = (1, 0.5)

  env = Environment()
  entity = env.create(Bot(x=0, y=0, mass=1, logic=logic, cup_capacity=0))
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
  id = env.create(Bot(x=0, y=0, mass=1, logic=reposition, cup_capacity=0))

  body = env.get(id, Body)

  assert body.position[0] == pytest.approx(0)
  assert body.position[1] == pytest.approx(0)

  env.step(1)

  assert body.position[0] == pytest.approx(0.1)
  assert body.position[1] == pytest.approx(0.1)
  assert body.angle == pytest.approx(3)
