## Lab 4 CUDA

### Escalera pedro, Iman Federico.

En este laboratorio utilizaremos CUDA para poder ejecutar tiny_mc en una GPU de Nvidia, en lugar de la CPU. El mayor beneficio de hacerlo es que la GPU es mucho más rápida que la CPU cuando se trata de problemas paralelizables.

El codigo lo ejecutaremos en zx81, que cuenta con una GPU Nvidia GeForce RTX 3070.

#### Roofline

Para obtener datos de nuestro programa usamos el profiler nsys nvprof en lugar de ncu, con el siguiente comando:

    /opt/cuda/11.2.2/bin/nsys nvprof ./tiny_mc

donde podemos ver 
- Estadisticas de las llamadas a funciones de la API de CUDA, y el tiempo que toman.
- Estadisticas de llamadas a kernels y el tiempo que toman.
- Estadisticas de operaciones de memoria y el tiempo que toman.
- Estadisticas de operaciones de memoria y el tamaño de memoria en KiB usado.

con Tiny.mc obtuvimos los siguiente resultados:

