#!/usr/bin/env python3
"""Summarize evolution timings from static and dynamic pthread binaries."""
import statistics
import sys
from collections import defaultdict


def main():
    if len(sys.argv) != 2:
        raise SystemExit(f"uso: {sys.argv[0]} arquivo.log")
    values = defaultdict(list)
    with open(sys.argv[1], encoding="utf-8") as source:
        for line in source:
            if not line.startswith("TIMING "):
                continue
            fields = dict(item.split("=", 1) for item in line.split()[1:])
            strategy = "dynamic" if "chunk" in fields else "static"
            key = (strategy, fields["rows"], fields["cols"],
                   fields["generations"], fields["threads"],
                   fields.get("chunk", "-"))
            values[key].append(float(fields["evolution_s"]))
    print("strategy,rows,cols,generations,threads,chunk,median_evolution_s,repeats")
    for key in sorted(values, key=lambda row: (row[1], row[2], row[3], row[0],
                                               int(row[4]), int(row[5]) if row[5] != "-" else 0)):
        print(",".join(key) + f",{statistics.median(values[key]):.9f},{len(values[key])}")


if __name__ == "__main__":
    main()
