#!/bin/bash
#SBATCH --job-name=life-speedup
#SBATCH --partition=sequana_cpu_dev
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=00:30:00
#SBATCH --output=speedup-%j.out

set -euo pipefail
cd "$SLURM_SUBMIT_DIR"

module load gcc/13.2_sequana
unset LIFE_TIMING
parallel_bin="${PARALLEL_BIN:-./parallel_static}"
speedup_output="${SPEEDUP_OUTPUT:-./bench-results/speedups.csv}"

python3 scripts/compare_life_speedup.py \
  --life ./life \
  --parallel "$parallel_bin" \
  --inputs ./bench-inputs \
  --output "$speedup_output" \
  --repeats 5
