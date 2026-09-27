"""Compara bytes de saída, incluindo espaços e quebras de linha."""
from pathlib import Path
import random
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def check(binary, name, data, expected):
    result = subprocess.run(
        [str(binary)], input=data, capture_output=True, timeout=15
    )
    if result.returncode != 0 or result.stdout != expected:
        raise AssertionError(
            f"{name}: exit={result.returncode}\n"
            f"esperado: {expected!r}\nobtido: {result.stdout!r}\n"
            f"stderr: {result.stderr!r}"
        )


def synthetic(rows, cols, generations, rng):
    # oraculo simples, independente do cálculo otimizado em C.
    rules = [(set(), set()), ({0, 3}, {2, 3}), (set(range(9)), {1, 8})]
    ids = [[rng.randrange(len(rules)) for _ in range(cols)] for _ in range(rows)]
    grid = [[rng.randrange(2) for _ in range(cols)] for _ in range(rows)]
    lines = [f"{rows} {cols} {generations}", str(len(rules))]
    lines += ["B" + "".join(map(str, sorted(b))) + "/S" +
              "".join(map(str, sorted(s))) for b, s in rules]
    lines += [" ".join(map(str, row)) for row in ids]
    lines += ["".join("x" if cell else " " for cell in row) for row in grid]
    data = ("\n".join(lines) + "\n").encode()
    for _ in range(generations):
        following = [[0] * cols for _ in range(rows)]
        for row in range(rows):
            for col in range(cols):
                neighbors = sum(
                    grid[r][c]
                    for r in range(max(0, row - 1), min(rows, row + 2))
                    for c in range(max(0, col - 1), min(cols, col + 2))
                    if (r, c) != (row, col)
                )
                birth, survival = rules[ids[row][col]]
                following[row][col] = neighbors in (survival if grid[row][col] else birth)
        grid = following
    expected = ("\n".join(
        "".join("x" if cell else " " for cell in row) for row in grid
    ) + "\n").encode()
    return data, expected


def main():
    binary = Path(sys.argv[1]).resolve()
    count = 0
    for index in range(1, 5):
        source = ROOT / f"life-{index}.in"
        check(binary, source.name, source.read_bytes(), source.with_suffix(".out").read_bytes())
        count += 1
    rng = random.Random(5645)
    for rows, cols in [(1, 1), (1, 7), (7, 1), (2, 2), (5, 7), (8, 11)]:
        for generations in [0, 1, 2, 9]:
            data, expected = synthetic(rows, cols, generations, rng)
            check(binary, f"{rows}x{cols}, G={generations}", data, expected)
            count += 1
    print(f"OK: {count} casos (4 fornecidos + 24 sintéticos), comparação byte a byte.")


if __name__ == "__main__":
    main()
