from arena.the_cherry_on_the_cake import *
from arena import *
import pytest

@pytest.fixture
def env():
    return Environment()

@pytest.fixture
def bot_entity(env):
    return env.create(Bot(x=-1000, y=0, mass=1))

@pytest.fixture
def bot_storage(env, bot_entity):
    env.attach(bot_entity, Storage(3))
    return env.get(bot_entity, Storage)

def test_pick_and_drop_cake_layer(env, bot_entity, bot_storage):
  layer_entity = env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING))

  bot_storage.store(layer_entity)
  assert list(bot_storage.owned) == [layer_entity]
  
  bot_storage.remove(layer_entity)
  assert list(bot_storage.owned) == []

def test_pick_cake_stack(env, bot_entity, bot_storage):
    env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING, stack=3))

    assert bot_storage.pick(x=0, y=0) != Entity.NULL
    assert bot_storage.pick(x=50, y=0) != Entity.NULL
    assert bot_storage.pick(x=100, y=0) == Entity.NULL

    assert len(list(bot_storage.owned)) == 2

pytest.main([__file__])
