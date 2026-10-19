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

python3 scripts/compare_life_speedup.py \
  --life ./life \
  --parallel ./parallel_static \
  --inputs ./bench-inputs \
  --output ./bench-results/speedups.csv \
  --repeats 5
