#!/bin/bash
#####  Constructed by HPC everywhere #####
#SBATCH --mail-user=abbajpai@iu.edu
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --time=0-02:00:00
#SBATCH --mem=503gb
#SBATCH --partition=largememory
#SBATCH --mail-type=FAIL,BEGIN,END
#SBATCH --job-name=my_job
#SBATCH --output=%j_output.txt
#SBATCH --error=%j_errors.txt


######  Module commands #####
module load python/3.9.8

######  Job commands go below this line #####
python3 Subset_Sum_Serial_Python.py
