#!/bin/bash
#SBATCH --job-name=life-dynamic
#SBATCH --partition=sequana_cpu_dev
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=00:20:00
#SBATCH --output=dynamic-%j.out

set -euo pipefail
cd "$SLURM_SUBMIT_DIR"
module load gcc/13.2_sequana
make parallel_static parallel_dynamic
mkdir -p bench-inputs bench-results

inputs=()
for spec in "500 500 100" "1000 1000 100" "2000 2000 100"; do
  read -r rows cols gens <<< "$spec"
  input="bench-inputs/${rows}x${cols}_G${gens}.in"
  python3 scripts/generate_bench_input.py "$rows" "$cols" "$gens" "$input"
  inputs+=("$input")
done

raw="bench-results/dynamic-static-${SLURM_JOB_ID}.log"
: > "$raw"
export LIFE_TIMING=1
for input in "${inputs[@]}"; do
  for threads in 1 2 4 8 16 24 48; do
    for run in 1 2 3 4 5; do
      ./parallel_static "$threads" < "$input" > /dev/null 2>> "$raw"
      for chunk in 1 2 4 8 16 32; do
        ./parallel_dynamic "$threads" "$chunk" < "$input" > /dev/null 2>> "$raw"
      done
    done
  done
done
python3 scripts/summarize_parallel_timings.py "$raw" > "${raw%.log}.csv"
echo "Medianas gravadas em ${raw%.log}.csv"
echo "Tempos gravados em $raw"
