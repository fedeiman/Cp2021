import subprocess
from datetime import datetime
import random
import sys
import os

def execute(command):
    return subprocess.run(command + " >> /dev/null" , shell=True)

def executeWithOutput(command):
    completedProc = subprocess.run(command, shell=True, capture_output=True)
    return completedProc.stdout

def runScale():
    path = sys.path[0] + '/'
    #randomSeed = random.randint(0, 4294967295)
    results = []
    execute(f'cd {path} && make clean')
    for i in range(32768, 327680, 32768):
        execute(f'cd {path} && make cleanMain')
        execute(f"cd {path} && make EXTRA='-DPHOTONS={i}'")
        temp = []
        for j in range(3):
            temp.append(float(executeWithOutput(f'cd {path} && ./tiny_cuda_pro')))
        results.append([i, sum(temp)/len(temp)])
    with open(f"{path}/results_sca_tinyCudav2.txt", "a") as f:
        f.write(f"\n\nResults on {datetime.now()}\n")
        f.write(str(results))
        f.write("\n")

if __name__ == "__main__":
    runScale()
