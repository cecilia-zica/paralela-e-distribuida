#!/usr/bin/env python3
"""Compare life.c and the pthread executable on identical input files."""

import argparse
import csv
import os
import random
import shlex
import statistics
import subprocess
import time
from pathlib import Path


THREAD_COUNTS = (1, 2, 4, 8, 16, 24, 48)


def run_once(command, input_path, env):
    with input_path.open("rb") as source:
        start = time.perf_counter()
        subprocess.run(command, stdin=source, stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL, check=True, env=env)
        end = time.perf_counter()
    return end - start


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--life", required=True, type=Path,
                        help="executavel compilado de life.c")
    parser.add_argument("--parallel", required=True, type=Path,
                        help="executavel parallel_static")
    parser.add_argument("--inputs", required=True, type=Path,
                        help="diretorio com os mesmos inputs usados no benchmark")
    parser.add_argument("--output", required=True, type=Path,
                        help="caminho do CSV de resultados")
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--seed", type=int, default=5645)
    args = parser.parse_args()

    if args.repeats < 1:
        parser.error("--repeats deve ser positivo")
    if not args.life.is_file() or not args.parallel.is_file():
        parser.error("nao encontrei um dos executaveis informados")

    env = os.environ.copy()
    env.pop("LIFE_TIMING", None)
    parallel_extra_args = shlex.split(env.get("PARALLEL_EXTRA_ARGS", ""))
    rng = random.Random(args.seed)
    records = []

    for input_path in sorted(args.inputs.glob("*.in")):
        with input_path.open("r", encoding="ascii") as source:
            rows, cols, generations = map(int, source.readline().split())

        life_command = [str(args.life.resolve())]
        parallel_commands = {
            threads: [str(args.parallel.resolve()), str(threads), *parallel_extra_args]
            for threads in THREAD_COUNTS
        }

        # Uma execucao de aquecimento para cada binario nao entra nas medianas.
        run_once(life_command, input_path, env)
        run_once(parallel_commands[1], input_path, env)

        samples = {("life", 1): []}
        samples.update({("parallel", threads): []
                        for threads in THREAD_COUNTS})
        for _ in range(args.repeats):
            trials = [("life", 1, life_command)]
            trials.extend(("parallel", threads, command)
                          for threads, command in parallel_commands.items())
            rng.shuffle(trials)
            for program, threads, command in trials:
                samples[(program, threads)].append(
                    run_once(command, input_path, env))

        life_median = statistics.median(samples[("life", 1)])
        for threads in THREAD_COUNTS:
            parallel_median = statistics.median(samples[("parallel", threads)])
            records.append({
                "rows": rows,
                "cols": cols,
                "generations": generations,
                "threads": threads,
                "repeats": args.repeats,
                "life_median_s": life_median,
                "parallel_median_s": parallel_median,
                "speedup": life_median / parallel_median,
            })

    args.output.parent.mkdir(parents=True, exist_ok=True)
    fields = ("rows", "cols", "generations", "threads", "repeats",
              "life_median_s", "parallel_median_s", "speedup")
    with args.output.open("w", newline="", encoding="ascii") as destination:
        writer = csv.DictWriter(destination, fieldnames=fields)
        writer.writeheader()
        for record in records:
            writer.writerow({key: (f"{value:.9f}" if isinstance(value, float)
                                   else value)
                             for key, value in record.items()})

    print(f"Resultados gravados em {args.output}")


if __name__ == "__main__":
    main()
