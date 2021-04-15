import sys, getopt
import statistics
import pprint
import random
from extra import execute, executeWithOutput, powerset, printProgressBar
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from scipy.interpolate import UnivariateSpline

import numpy as np

def runExplore(linux = True, runs=10, photons=32768):
    # Fixed random seed between runs to avoid program differences in runtime
    randomSeed = random.randint(0, 4294967295)
    # Different versions of tiny
    targets = ["tiny_mc", "tiny_twis"]
    # March native no funca en M1 y gcc se llama diferente
    if linux:
        compilers = ["gcc", "clang"]
        flags1 = ['-O1', '-O2', '-O3','-Ofast']
        flags2 = [' -ffast-math ', ' -march=native ', ' -funroll-loops ', ' -funroll-all-loops ']

    else:
        compilers = ["gcc-10", "clang"]
        flags1 = ['-O1', '-O2', '-O3','-Ofast']
        flags2 = [' -ffast-math ']

    powerFlagsw2 =[''.join(tup) for tup in powerset(flags2)] #Crete powerset of flags2
    superPowrFlags = [fl1 + fl2 for fl1 in flags1 for fl2 in powerFlagsw2] #Get all permutations of the powerset and Os


    # Pa guardar resultados para cada target posible
    targetResults = [[] for _ in targets]

    progressTotal = len(compilers) * len(targets) * len(superPowrFlags)
    progressCur = 0
    ##Compile first
    for  compiler in compilers:
        for index, flag in enumerate(superPowrFlags):
            for target in targets:
                #Limpia
                execute(f'make cleanMain TARGET={target}')
                # Arma comando
                compiler_command = f"make CC='{compiler}' EXTRA='{flag} -DPHOTONS={photons} -DSEED={randomSeed}' TARGET='{target}' -j8  "
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
                meanResult = {"command":command ,"value" : round(statistics.mean(results), 2), "compileCommand":compiler_command}
                #Turbion pero todo tiene sentido 
                targetResults[targets.index(target)].append(meanResult)
                progressCur += 1
                printProgressBar(progressCur, progressTotal)


    with open("results.txt", "a") as f:
        if linux:
            f.write("\n\nResults for Intel Core i7\n") 
        else:
            f.write("\n\nResults for Apple M1\n") 
        for index,result in enumerate(targetResults):
            result.sort(key = lambda i: i['value'], reverse=True)
            f.write(f"\nResults for {targets[index]}\n")
            #Ordena las listas tomando en cuenta el value nomas, prettyprintealo al files
            pprint.pprint([{key:d[key] for key in d if key!='compileCommand'} for d in result], f)
    # Dame el commando con el mejor tiempo
    return max([results[0] for results in targetResults],key=lambda x:x['value'])["compileCommand"]

def runScale(linux=True, flags='-O2 -ffast-math -funroll-all-loops', compiler='gcc-10', target='tiny_twis'):
    randomSeed = random.randint(0, 4294967295)
    results = []
    for i in range(0, 131073, 2048):
        execute(f'make cleanMain TARGET={target}')
        execute(f"make CC='{compiler}' EXTRA='{flags} -DPHOTONS={i} -DSEED={randomSeed}' TARGET='{target}'")
        temp = []
        for _ in range(5):
            temp.append(float(executeWithOutput(f'./{target}')))
        results.append([i, statistics.mean(temp)])
        printProgressBar(i, 131073)
    x, y = zip(*results)
    #Scipy spline interpolation
    spl = UnivariateSpline(x, y, k=2)
    xs = np.linspace(0, 131072, 500)
    plt.plot(xs, spl(xs), 'g', lw = 2)
    plt.scatter(x, y, s=10)
    plt.savefig('Result.png')
    

def main(argv):
    linux = True
    scale = False
    explore = False
    expResults = ""
    try:
        opts, _ = getopt.getopt(argv,"h",["linux","macos", "explore", "scale", "help"])
    except getopt.GetoptError:
        print('usage: [--linux || --macos && --explore || --scale] ')
        print('Example: ./tester.py --linux --explore')
        sys.exit(2)
    for opt, _ in opts:
        if opt in ('-h', '--help'):
            print('usage: [--linux || --macos && --explore || --scale] ')   
            print('Example: ./tester.py --linux --explore')
            sys.exit()
        elif opt == "--linux":
            linux = True
        elif opt == "--macos":
            linux = False
        elif opt == "--explore":
            explore = True
        elif opt == "--scale":
            scale = True
    if explore:
        expResults = runExplore(linux)
    if scale:
        if expResults:
            runScale(linux, expResults)
        else:
            runScale(linux)
   
if __name__ == "__main__":
   main(sys.argv[1:])