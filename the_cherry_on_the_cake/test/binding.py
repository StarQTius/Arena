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
def bot_cake_storage(env, bot_entity):
    env.attach(bot_entity, CakeStorage(3))
    return env.get(bot_entity, CakeStorage)

def test_pick_and_drop_cake_layer(env, bot_entity, bot_cake_storage):
  layer_entity = env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING))

  bot_cake_storage.store(layer_entity)
  assert list(bot_cake_storage.owned) == [layer_entity]
  
  bot_cake_storage.remove(layer_entity)
  assert list(bot_cake_storage.owned) == []

def test_pick_cake_stack(env, bot_entity, bot_cake_storage):
    env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING, stack=3))

    assert bot_cake_storage.pick(x=0, y=0) != Entity.NULL
    assert bot_cake_storage.pick(x=50, y=0) != Entity.NULL
    assert bot_cake_storage.pick(x=100, y=0) == Entity.NULL

    assert len(list(bot_cake_storage.owned)) == 2

def test_rearrange_stack(env, bot_entity, bot_cake_storage):
    env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING, stack=3))
    cake_layer_entities = [bot_cake_storage.pick(x=0, y=0) for _ in range(3)]

    env.get(cake_layer_entities[0], Body).set_position((0, 0))
    bot_cake_storage.remove(cake_layer_entities[0])

    env.get(cake_layer_entities[2], Stackable).stack(cake_layer_entities[0])
    bot_cake_storage.remove(cake_layer_entities[2])

    env.get(cake_layer_entities[1], Stackable).stack(cake_layer_entities[2])
    bot_cake_storage.remove(cake_layer_entities[1])

    assert len(list(bot_cake_storage.owned)) == 0
    assert env.get(cake_layer_entities[1], Stackable).range == [cake_layer_entities[1], cake_layer_entities[2], cake_layer_entities[0]]
    assert [*map(lambda x : env.get(x, Body).is_enabled, env.get(cake_layer_entities[1], Stackable).range)] == [True, False, False]

def test_stack_cherry_on_cake_layer(env, bot_entity):
    cake_layer_entity = env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING))
    cherry_entity = env.create(Cherry(x=500, y=0))

    env.get(cherry_entity, Stackable).stack(cake_layer_entity)

    assert (env.get(cherry_entity, Body).position == (0, 0)).all()

def test_cannot_stack_anything_on_cherry(env, bot_entity):
    cake_layer_entity = env.create(CakeLayer(x=0, y=0, flavor=Flavor.ICING))
    cherry1_entity = env.create(Cherry(x=500, y=0))
    cherry2_entity = env.create(Cherry(x=1000, y=0))

    with pytest.raises(RuntimeError):
        env.get(cake_layer_entity, Stackable).stack(cherry1_entity)

    with pytest.raises(RuntimeError):
        env.get(cherry2_entity, Stackable).stack(cherry1_entity)
