from arena import *
from arena.sail_the_world import *
import pytest

def test_create_a_cup_and_move_it_with_a_bot():
  def move_forward(body: Body):
    body.velocity = (1, 0)

  env = Environment()
  env.create(Bot(x=-1.5, y=0, mass=1, logic=move_forward))
  id = env.create(Cup(x=1, y=0, color=CupColor.RED))
  for _ in range(500):
    env.step(0.1)

  body = env.get(id, Body)

  for cup in cups(env):
    assert body.position[0] > 1
    assert body.position[1] == pytest.approx(0)

def test_filter_cups_range():
  def distance_squared(body_a, body_b):
    return (body_a.position[0] - body_b.position[0]) ** 2 + (body_a.position[1] - body_b.position[1]) ** 2

  def grab(env: Environment, body: Body, cup_grabber: CupGrabber):
    inrange_cups = filter(lambda x: distance_squared(body, x[1]) < 4, cups(env))
    cup_grabber.grab(next(inrange_cups)[0])

  env = Environment()
  id = env.create(Bot(x=-1.5, y=0, mass=1, logic=grab))
  env.attach(id, CupGrabber(capacity=2))
  env.create(Cup(x=0, y=0, color=CupColor.RED))
  env.create(Cup(x=1, y=0, color=CupColor.GREEN))
  env.step(1)
  with pytest.raises(Exception):
    env.step(1)

  assert sum(map(lambda _ : 1, cups(env))) == 1
  assert next(cups(env))[2] == CupColor.GREEN

def test_grab_and_drop():
  def grab_n_drop(env: Environment, body: Body, cup_grabber: CupGrabber):
    nonlocal flag
    if not flag:
      cup_grabber.grab(next(cups(env))[0])
    else:
      cup_grabber.drop(Cup(x=0.5, y=0, color=CupColor.RED))
    flag = True

  flag = False

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1, logic=grab_n_drop))
  env.attach(id, CupGrabber(capacity=2))
  env.create(Cup(x=0, y=0, color=CupColor.RED))
  env.step(1)

  assert sum(map(lambda _ : 1, cups(env))) == 0

  env.step(1)

  assert sum(map(lambda _ : 1, cups(env))) == 1

  _, body, color = next(cups(env))

  assert body.position[0] == 0.5
  assert body.position[1] == 0
  assert color == CupColor.RED

def test_access_cup_grabber_storage():
  def grab_all(env: Environment, cup_grabber: CupGrabber):
    nonlocal red_count, green_count
    for entity, _0, _1 in cups(env):
      cup_grabber.grab(entity)
    red_count = cup_grabber.get_count(CupColor.RED)
    green_count = cup_grabber.get_count(CupColor.GREEN)

  red_count = 0
  green_count = 0

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1, logic=grab_all))
  env.attach(id, CupGrabber(capacity=5))
  env.create(Cup(x=0, y=0, color=CupColor.RED))
  env.create(Cup(x=0.2, y=0, color=CupColor.GREEN))
  env.create(Cup(x=0.4, y=0, color=CupColor.RED))
  env.create(Cup(x=0.6, y=0, color=CupColor.GREEN))
  env.create(Cup(x=0.8, y=0, color=CupColor.RED))
  env.step(1)

  assert sum(map(lambda _ : 1, cups(env))) == 0
  assert red_count == 3
  assert green_count == 2

def test_get_the_content_of_an_empty_cup_grabber():
  def access_storage(cup_grabber: CupGrabber):
    cup_grabber.get_count(CupColor.RED)

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1, logic=access_storage))
  env.attach(id, CupGrabber(capacity=5))
  env.step(1)

def test_create_a_field_of_custom_dimension():
  def go_forward(body: Body):
    body.forward_velocity = 1

  env = Environment(width=1, height=1)
  id = env.create(Bot(x=-1, y=0, mass=1, logic=go_forward))
  body = env.get(id, Body)
  for _ in range(5):
    env.step(1)

  assert sum(map(lambda _ : 1, cups(env))) == 0
  assert -0.5 < body.position[0] < 0.5
  assert -0.5 < body.position[1] < 0.5
