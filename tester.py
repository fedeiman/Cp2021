import subprocess
import threading
import os
from time import sleep

def execute(command):
    return subprocess.run(command, shell=True)

userInput = input('Linux, Y, N ?')
if userInput == 'y' or userInput == 'yes' or userInput == 'si' or userInput == 's':
    linux = True
else:
    linux = False

photons = 32768
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


results = []
##Compile first
for  compiler in compilers:
    for index, flag in enumerate(flags):
        execute('make clean')
        compiler_command = f"make CC='{compiler}' EXTRA='{flag} -DPHOTONS={photons}' "
        execute(compiler_command)
        execute(f'./tiny_mc >> results_{compiler}_{index}.txt')



