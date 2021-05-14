import subprocess
from datetime import datetime
import random
import sys

def execute(command):
    return subprocess.run(command + " >> /dev/null" , shell=True)

def executeWithOutput(command):
    completedProc = subprocess.run(command, shell=True, capture_output=True)
    return completedProc.stdout

def runScale():
    randomSeed = random.randint(0, 4294967295)
    results = []
    for i in range(512, 65538, 512):
        execute(f'make cleanMain')
        execute(f"make -DPHOTONS={i} -DSEED={randomSeed}'")
        temp = []
        for j in range(5):
            if j> 0:
                temp.append(float(executeWithOutput(f'./tiny_mc')))
        results.append([i, sum(temp)/len(temp)])
    scriptPath = sys.path[0]
    with open("scriptPath/results.txt", "a") as f:
        f.write(f"\n\nResults on {datetime.now()}\n")
        f.write(results)
        f.write("\n")

if __name__ == "__main__":
    execute('make clean')
    runScale()