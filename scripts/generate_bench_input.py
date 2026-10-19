#!/usr/bin/env python3
"""Generate a reproducible Life-like input for SDumont timing runs."""

import argparse
import random


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("rows", type=int)
    parser.add_argument("cols", type=int)
    parser.add_argument("generations", type=int)
    parser.add_argument("output")
    parser.add_argument("--seed", type=int, default=5645)
    parser.add_argument("--density", type=float, default=0.30)
    args = parser.parse_args()

    if args.rows <= 0 or args.cols <= 0 or args.generations < 0:
        parser.error("rows/cols devem ser positivos e generations >= 0")
    if not 0.0 <= args.density <= 1.0:
        parser.error("density deve estar entre 0 e 1")

    rng = random.Random(args.seed)
    with open(args.output, "w", encoding="ascii", newline="\n") as out:
        out.write(f"{args.rows} {args.cols} {args.generations}\n1\nB3/S23\n")
        rule_row = " ".join(["0"] * args.cols)
        for _ in range(args.rows):
            out.write(rule_row + "\n")
        for _ in range(args.rows):
            row = "".join("x" if rng.random() < args.density else " "
                          for _ in range(args.cols))
            out.write(row + "\n")


if __name__ == "__main__":
    main()
