import asyncio
import time
from threading import Thread
import os

import pytest
import websockets


total_runs = int(os.getenv("WS_TEST_TOTAL_RUNS", 10))
max_threads = int(os.getenv("WS_TEST_N_THREADS", 32))
results = []


async def run(n_tests):
    async with websockets.connect("ws://127.0.0.1:8080") as websocket:
        for _ in range(n_tests):
            data = "HELLO"
            await websocket.send(data)
            print("sent %d" % len(data))
            result = await websocket.recv()
            print('received %d: "%s"' % (len(result), result))
            await asyncio.sleep(0.1)


def runner(n_tests):
    success = True
    try:
        loop = asyncio.new_event_loop()
        loop.run_until_complete(run(n_tests))
    except Exception as e:
        success = False
    results.append(success)


def test_multi_thread():
    threads = []
    for _ in range(max_threads):
        threads.append(Thread(target=runner, args=(total_runs,)))

    [t.start() for t in threads]
    [t.join() for t in threads]

    assert all(results)
