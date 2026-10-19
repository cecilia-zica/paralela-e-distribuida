#!/usr/bin/env python3
"""Print medians from LIFE_TIMING lines as CSV."""

import statistics
import sys
from collections import defaultdict


def main():
    if len(sys.argv) != 2:
        raise SystemExit(f"uso: {sys.argv[0]} bench-results/timings.log")

    samples = defaultdict(lambda: defaultdict(list))
    with open(sys.argv[1], encoding="utf-8") as source:
        for line in source:
            if not line.startswith("TIMING "):
                continue
            fields = dict(item.split("=", 1) for item in line.split()[1:])
            key = tuple(fields[name] for name in
                        ("rows", "cols", "generations", "threads"))
            for name in ("input_s", "allocation_s", "evolution_s", "print_s", "total_s"):
                samples[key][name].append(float(fields[name]))

    print("rows,cols,generations,threads,input_median_s,allocation_median_s,"
          "evolution_median_s,print_median_s,total_median_s")
    for key in sorted(samples, key=lambda item: tuple(map(int, item))):
        medians = [statistics.median(samples[key][name]) for name in
                   ("input_s", "allocation_s", "evolution_s", "print_s", "total_s")]
        print(",".join(key) + "," + ",".join(f"{value:.9f}" for value in medians))


if __name__ == "__main__":
    main()
