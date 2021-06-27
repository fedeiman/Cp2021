## Lab 4 CUDA

### Escalera pedro, Iman Federico.

En este laboratorio utilizaremos CUDA para poder ejecutar tiny_mc en una GPU de Nvidia, en lugar de la CPU. El mayor beneficio de hacerlo es que la GPU es mucho más rápida que la CPU cuando se trata de problemas paralelizables.

El codigo lo ejecutaremos en zx81, que cuenta con una GPU Nvidia GeForce RTX 3070.


#### Estrategias intentadas
Tomamos la primer aproximacion mas intuitiva que fue simplemente hacer arreglos en la memoria unificada y lanzar cada foton en un thread individual obtuviendo asi un rendimiento muy superior a los resultados obtenidos por cpu en los labs anteriores. Cada uno de estos threads modifica atomicamente los arreglos lo que repercute negativamente en el rendimiento.

            atomicAdd(&result.heat[shell], ((1.0f - albedo) * weight));
            atomicAdd(&result.heat2[shell], ((1.0f - albedo) * (1.0f - albedo) * weight * weight));

Para esta implementacion usamos el generador de numeros 
aleatorios de CUDA curand, ya que tiene buenas propidades estadisticas y de performance para GPGPU. 

Con esta implementacion logramos una mejora de aproximadamente 4x comparado con el mejor resultado obtenido en cpu con 23 threads.


La segunda implementacion que realizamos fue con la ayuda del paper [CUDAMCML](https://www.atomic.physics.lu.se/fileadmin/atomfysik/Biophotonics/Software/CUDAMCML.pdf)
en la cual se explicaban varias optimizaciones para el problema de las cuales estaba una que nos parecio interesante y ayudaba con el problema de las operaciones atomicas.
Esta se basaba en un fenomeno conocido del problema de tiny_mc donde en general se suma heat al mismo shell sucesivamente, esto pasa cuando la cantidad de shells es poca (101). Lo primero que hicimos fue verificar si esto sucede, y comprobamos que si lo hace, aproximadamente en un 30 a 50% cada vez que se escribe en los arreglos de heat.
Esto nos ayuda con el problema de las operaciones atomicas ya que podemos agregar una variable que funciona a modo de cache y vaya acumulando la suma siempre que se deba aumentar calor de forma sucesiva  




#### Roofline

Para obtener datos de nuestro programa usamos el profiler nsys nvprof en lugar de ncu, con el siguiente comando:

    /opt/cuda/11.2.2/bin/nsys nvprof ./tiny_mc

donde podemos ver 
- Estadisticas de las llamadas a funciones de la API de CUDA, y el tiempo que toman.
- Estadisticas de llamadas a kernels y el tiempo que toman.
- Estadisticas de operaciones de memoria y el tiempo que toman.
- Estadisticas de operaciones de memoria y el tamaño de memoria en KiB usado.

con Tiny.mc obtuvimos los siguiente resultados:

