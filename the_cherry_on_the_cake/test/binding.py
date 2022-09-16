from arena.the_cherry_on_the_cake import *
from arena import *
import pytest

def test_pick_and_drop_cake_stack():
  env = Environment()
  bot_entity, layer_entity = env.create(Bot(x=-1000, y=0, mass=1)), env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING))
  env.attach(bot_entity, Storage(3))
  bot_storage = env.get(bot_entity, Storage)

  bot_storage.store(layer_entity)

  assert list(bot_storage.owned) == [layer_entity]

  bot_storage.remove(layer_entity)
  
  assert list(bot_storage.owned) == []
