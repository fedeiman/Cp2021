import subprocess
import threading
import os
from time import sleep
import statistics
import pprint

def execute(command):
    return subprocess.run(command, shell=True)

def executeWithOutput(command):
    completedProc = subprocess.run(command, shell=True, capture_output=True)
    return completedProc.stdout

userInput = input('Linux, Y, N ?')
if userInput == 'y' or userInput == 'yes' or userInput == 'si' or userInput == 's':
    linux = True
else:
    linux = False

runs = 10
# Redifinable phtons
photons = 32768
# Different versions of tiny
targets = ["tiny_mc", "tiny_twis"]
# March native no funca en M1
if linux:
    compilers = ["gcc", "clang"]
    flags = ['-O1', '-O2', '-O3', '-O2 -ffast-math', '-O1 -ffast-math' ,
            '-O3 -ffast-math', '-O2 -march=native', '-O3 -march=native', '-O1 -march=native',
            '-O2 -ffast-math -march=native', '-O3 -ffast-math -march=native', '-O1 -ffast-math -march=native',
            '-O3 -ffast-math -march=native -funroll-loops', '-O2 -ffast-math -march=native -funroll-loops',
            '-O2 -funroll-loops', '-O3 -funroll-loops', '-O3 -ffast-math -funroll-loops', '-O2 -ffast-math -funroll-loops',
            '-O3 -march=native -funroll-loops',  '-O2 -march=native -funroll-loops', '-Ofast']
else:
    compilers = ["gcc-10", "clang"]
    flags = ['-O1', '-O2', '-O3', '-O2 -ffast-math', '-O1 -ffast-math', 
            '-O2 -funroll-loops', '-O3 -funroll-loops', '-O3 -ffast-math -funroll-loops', '-O2 -ffast-math -funroll-loops', '-Ofast']


# Pa guardar resultados para cada target posible
targetResults = [[] for _ in targets]

##Compile first
for  compiler in compilers:
    for index, flag in enumerate(flags):
        for target in targets:
            #Limpia
            execute('make clean')
            # Arma comando
            compiler_command = f"make CC='{compiler}' EXTRA='{flag} -DPHOTONS={photons}' TARGET='{target}'  "
            #Compila
            execute(compiler_command)
            results = []
            #Corre varias veces (10?)
            for run in range(runs):
                result = float(executeWithOutput(f'./{target}'))
                # Pa calentar la cache? solo guardamos las ultimas 6 corridas
                if run > 4:
                    results.append(result)
            # Guardo el resultado en la lista como un dict para que sea mas facil
            command = f'{compiler}_{flag}'
            meanResult = {"command":command ,"value" : round(statistics.mean(results), 2)}
            #Turbion pero todo tiene sentido 
            targetResults[targets.index(target)].append(meanResult)


with open("results.txt", "a") as f:
    if linux:
        f.write("Results for Intel Core i7\n") 
    else:
        f.write("Results for Apple M1\n") 
    for index,result in enumerate(targetResults):
        f.write(f"\nResults for {targets[index]}\n")
        #Ordena las listas tomando en cuenta el value nomas, prettyprintealo al files
        pprint.pprint((sorted(result, key = lambda i: i['value'], reverse=True)), f)
    
    



