#!/usr/bin/env python3
import asyncio
import _datetime
from datetime import datetime, timedelta
from typing import AsyncIterable

from yapapi import Golem, Task, WorkContext
from yapapi.log import enable_default_logger
from yapapi.payload import vm
from yapapi.strategy import (
    MarketStrategy, SCORE_NEUTRAL, SCORE_TRUSTED,
    SCORE_REJECTED
)

#not in use
class MyStrategy(MarketStrategy):
    async def score_offer(
            self, offer, x
    ) -> float:
        """Score `offer`. Better offers should get higher scores."""
        now = _datetime.date.today()
        #current_time = now.strftime("%H:%M:%S")
        print("Current Time =", now)
        print(f"offer id: {offer.id}")
        print(f"offer issuer: {offer.issuer}")
        print(f"offer props: {offer.props}")
        # if offer.issuer == "0xa4eb429ff594dd59753d6994c14928b1b994175c":
        #    return SCORE_TRUSTED
        if offer.props["golem.inf.cpu.threads"] > 1:
            return SCORE_TRUSTED
        return SCORE_REJECTED


async def worker(context: WorkContext, tasks: AsyncIterable[Task]):
    async for task in tasks:
        script = context.new_script()
        script.run("/bin/sh","-c","./mbf9rng " + str(task.data) + " 300 1 >> result.bin")
        script.run("/bin/sh","-c","cp result.bin /golem/output")
        future_result = script.download_file("/golem/output/result.bin", "result" + str(task.data) + ".bin")
        yield script
        task.accept_result(result=await future_result)


async def main():
    package = await vm.repo(
        image_hash="324fcfaf42b3c34428fc6f804a1b4cca17560a7dc64c0546573004e4",
    )

    tasks = []
    for i in range (0, 2):
        tasks.append(Task(data=1026964 + i*30))

#    async with Golem(budget=1.0, subnet_tag="public", strategy=MyStrategy()) as golem:
    async with Golem(budget=100.0, subnet_tag="public") as golem:
        async for completed in golem.execute_tasks(worker, tasks, payload=package, max_workers=50, timeout=timedelta(minutes=120)):
            print(completed.result.stdout)


if __name__ == "__main__":
    enable_default_logger(log_file="hello.log")

    loop = asyncio.get_event_loop()
    task = loop.create_task(main())
    loop.run_until_complete(task)
