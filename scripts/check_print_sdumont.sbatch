#!/bin/bash
#SBATCH --job-name=life-print-check
#SBATCH --partition=sequana_cpu_dev
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=00:10:00
#SBATCH --output=print-check-%j.out

set -euo pipefail
cd "$SLURM_SUBMIT_DIR"

before="./parallel_static_before_print"
after="./parallel_static_after_print"
out_dir="bench-results/print-check"
mkdir -p "$out_dir"

for input in bench-inputs/*.in; do
  name=${input##*/}
  "$before" 24 < "$input" > "$out_dir/$name.before.out"
  "$after" 24 < "$input" > "$out_dir/$name.after.out"
  cmp "$out_dir/$name.before.out" "$out_dir/$name.after.out"
  echo "OK: $name"
done
