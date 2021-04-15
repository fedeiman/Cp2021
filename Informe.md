## Informe Laboratorio 1

#### Hardware y software
- Intel® Core™ i7-8550U CPU @ 1.80GHz × 8 
- 16 gb de ram 
- Ubuntu 20.04.2 LTS x86-64
- gcc 9.3.0
- clang 10.0.0-4ubuntu1
- kernel 5.8.0-48-generic 
---------------------------------
- Apple M1 x 8 
- 16 gb ram
- gcc 10.2.1
- clang Apple clang 12.0.0
- macOS 11.2.3 ARM-64
- kernel 20.3.0

#### Metricas
Como metricas usamos dos, primero el resultado que obtenemos de fotones por segundo y luego usamos el comando 

    sudo perf stat -M FLOPS ./tiny_mc

para obtener los GFLOPs, pero solo en la maquina con el i7 ya que perf es dependiente de linux (los resultados son un promedio de 10 corridas).

Con el i7 y con gcc obtuvimos de promedio 0,70 GFLOPs  y 190mil fotones por segundo sin ningun tipo de optimizacion. optimizando con la flag -O3 el promedio subio a 0,95GFLOPs y 246mil fotones por segundo finalmente con -O3 -march=native, no obtuvimos una mejora ya que obtuvimos un promedio incluso menor que con -O3 (0,89 GFLOPs y 225mil fotones por segundo).

Por otro lado con clang sin ninguna optimizacion obtuvimos un promedio de 0,75 GFLOPs  y 190mil fotones por segundo. con -O3 obtuvimos 1,06 GFLOPs
pero solo 255mil fotones por segundo obteniendo una mejoria con respecto a gcc. con -O3 -march=native el promedio bajo a 0,88 GFLOPs y 221mil fotones por segundo, demostrando asi que el mejor resultado lo obtenemos con -O3 y clang 


Finalmente como optimizacion decidimos usar una libreria para generar numeros random mediante el conocido generador de numeros llamado [Mersenne twister](https://github.com/ESultanik/mtwister)

# resultados con mac van aca 
# Gráficas de scaling para la versión más rápida obtenida.

