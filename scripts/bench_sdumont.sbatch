#!/bin/bash
#SBATCH --job-name=life-timing
#SBATCH --partition=sequana_cpu_dev
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=00:30:00
#SBATCH --output=bench-%j.out

set -euo pipefail
cd "$SLURM_SUBMIT_DIR"

# Use a mesma versão registrada nas medições anteriores do projeto.
module load gcc/13.2.0
export LIFE_TIMING=1

mkdir -p bench-inputs bench-results
printf 'host=%s\ngcc=%s\ncpus=%s\n' \
  "$(hostname)" "$(gcc --version | head -n 1)" "$SLURM_CPUS_PER_TASK" \
  > bench-results/job-info.txt

# Três tamanhos de grade e dois valores de G; geração das entradas fica fora
# da medição do executável.
python3 scripts/generate_bench_input.py 500 500 10 bench-inputs/500x500_G10.in
python3 scripts/generate_bench_input.py 500 500 100 bench-inputs/500x500_G100.in
python3 scripts/generate_bench_input.py 1000 1000 10 bench-inputs/1000x1000_G10.in
python3 scripts/generate_bench_input.py 1000 1000 100 bench-inputs/1000x1000_G100.in
python3 scripts/generate_bench_input.py 2000 2000 10 bench-inputs/2000x2000_G10.in
python3 scripts/generate_bench_input.py 2000 2000 100 bench-inputs/2000x2000_G100.in

raw=bench-results/timings.log
: > "$raw"

for input in bench-inputs/*.in; do
  # Aquecimento e leitura inicial do arquivo, não contabilizados.
  ./parallel_static 1 < "$input" > /dev/null 2>/dev/null

  for threads in 1 2 4 8 16 24 48; do
    for run in 1 2 3 4 5; do
      ./parallel_static "$threads" < "$input" > /dev/null 2>> "$raw"
    done
  done
done

echo "Medições brutas: $SLURM_SUBMIT_DIR/$raw"
