from arena import *
import arena.sail_the_world as stw

def test_render_a_bot_and_some_cups():
  def move_forward(body: Body):
    body.velocity = (0.5, 0)

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1, logic=move_forward))  
  env.attach(id, Host(move_forward))
  env.create(stw.Cup(x=0, y=0.07, color=stw.CupColor.RED))
  env.create(stw.Cup(x=0, y=0, color=stw.CupColor.GREEN))
  env.create(stw.Cup(x=0, y=-0.07, color=stw.CupColor.RED))
  with env.renderer:
    for _ in range(300):
      env.step(0.01)

def test_render_a_bot_grabbing_a_cup():
  i = 2
  def grab(env: Environment, body: Body, cup_grabber: stw.CupGrabber):
    nonlocal i
    i -= 1
    if i == 0:
      cup_grabber.grab(next(env.cups)[0])

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1))
  env.attach(id, Host(grab))
  env.attach(id, stw.CupGrabber(capacity=2))
  env.create(stw.Cup(x=0, y=0, color=stw.CupColor.RED))
  with env.renderer:
    env.step(1)
    env.step(1)
    env.step(1)

def test_drive_a_bot_with_forward_velocity():
  def drive_n_turn(body: Body):
    body.angular_velocity = 5
    body.forward_velocity = 0.5

  env = Environment()
  id = env.create(Bot(x=-1, y=0, mass=1, logic=drive_n_turn))
  env.attach(id, Host(drive_n_turn))
  
  with env.renderer:
    for _ in range(60):
      env.step(1/20)
