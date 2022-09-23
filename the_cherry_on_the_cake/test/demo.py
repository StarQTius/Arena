import arena.the_cherry_on_the_cake as coc
from arena import *

def logic(body: Body, cake_storage: coc.CakeStorage, cherry_storage: coc.CherryStorage):
    body.forward_velocity = 500

    cake_storage.pick(x=body.position[0] + 250, y=0)
    cherry_storage.pick(x=body.position[0] + 250, y=0)

    if len([*cake_storage.owned]) == 1 and len([*cherry_storage.owned]) == 1:
        cake_layer_entity = next(cake_storage.owned)
        cherry_entity = next(cherry_storage.owned)

        env.get(cake_layer_entity, Body).set_position((500, 0))
        cake_storage.remove(cake_layer_entity)

        env.get(cherry_entity, Stackable).stack(cake_layer_entity)
        cherry_storage.remove(cherry_entity)

env = coc.default_layout()

with env.renderer:
    bot_entity = env.create(Bot(x=-1000, y=0, mass=1))
    env.attach(bot_entity, Host(logic))
    env.attach(bot_entity, coc.CakeStorage(3))
    env.attach(bot_entity, coc.CherryStorage(3))

    for _ in range(1000):
        env.step(1/200)
