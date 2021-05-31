#!/bin/bash

#SBATCH --job-name=tinyMC
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=23
export OMP_NUM_THREADS=23

srun ./scaling_lab2.py