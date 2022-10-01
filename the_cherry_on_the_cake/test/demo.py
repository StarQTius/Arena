import numpy as np

import arena.the_cherry_on_the_cake as coc
from arena import *

step = 0

def draw_ray(env, entity, x, y, angle, distance):
    env.renderer.draw_segment((x, y), np.array([x, y]) + distance * np.array([np.cos(angle), np.sin(angle)]))

def logic(body: Body, ray: Ray, cake_storage: coc.CakeStorage, cherry_storage: coc.CherryStorage):
    global step

    body.forward_velocity = 500
    ray.sweep(fov=np.pi / 1.5, definition=50)

    if step == 0:
        cake_storage.pick(x=body.position[0] + 250, y=0)
        cherry_storage.pick(x=body.position[0] + 250, y=0)

        if len([*cake_storage.owned]) == 1 and len([*cherry_storage.owned]) == 1:
            cake_layer_entity = next(cake_storage.owned)
            cherry_entity = next(cherry_storage.owned)

            env.get(cake_layer_entity, Body).set_position((1000, 0))
            cake_storage.remove(cake_layer_entity)

            env.get(cherry_entity, Stackable).stack(cake_layer_entity)
            cherry_storage.remove(cherry_entity)

            step = 1
            
    elif step == 1:
        try:
            cake_storage.pick(x=body.position[0] + 250, y=0)
        except RuntimeError:
            pass
        
        try:
            cherry_storage.pick(x=body.position[0] + 250, y=0)
        except RuntimeError:
            pass
        
        if len([*cake_storage.owned]) == 1 and len([*cherry_storage.owned]) == 1:
            step = 2

    elif step == 2:
        body.set_angle(np.pi)
        cake_layer_entity = next(cake_storage.owned)
        cherry_entity = next(cherry_storage.owned)
        env.get(cake_layer_entity, Body).set_position((0, 0))
        cake_storage.remove(cake_layer_entity)
        env.get(cherry_entity, Stackable).stack(cake_layer_entity)
        cherry_storage.remove(cherry_entity)
        env.get(cake_layer_entity, Body).set_position((0, -100))

        step = 3
        

env = coc.default_layout()

with env.renderer:
    bot_entity = env.create(Bot(x=-1000, y=0, mass=1))
    env.attach(bot_entity, Host(logic))
    env.attach(bot_entity, Ray(range=1500, filter=lambda e: env.all_of(e, [coc.CherryLike])))
    env.attach(bot_entity, coc.CakeStorage(3))
    env.attach(bot_entity, coc.CherryStorage(3))

    env.on_ray_fired(lambda *args : draw_ray(env, *args))

    for _ in range(150):
        env.step(1/20)
