import pytest

def test_zigzag():
    from arena import Environment, Bot, Host, Body, wait_next
    import numpy as np

    async def logic(body: Body):
        a = 0
        while True:
            body.forward_velocity = 1000
            body.angular_velocity = 5 * np.cos(a)
            a += 5e-2
            await wait_next()

    env = Environment()
    entity = env.create(Bot(x=-1000, y=0, mass=1))
    env.attach(entity, Host(logic))

    for _ in range(200):
        env.step(1e-2)
