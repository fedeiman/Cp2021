## Informe Laboratorio 2

### Integrantes: Escalera Pedro, Imán Federico.

#### Autovectorizacion

##### GCC:

-O3  o -ftree-vectorize activan la vectorizacion de loops pero no mostraran que optimizaron a menos que activemos otras flags como:

    -fopt-info-vec  -fopt-info-vec-optimized

El compilador mostrara que loops (por linea N°)
fueron optimizados.

    -fopt-info-vec-missed 

Detalla informacion sobre los loops que no fueron vectorizados entre otros datos.

    -fopt-info-vec-note 

Detalla informacion sobre todos los loops y las optimizaciones realizadas


    -fopt-info-vec-all

Todas las opciones anteriores juntas.

Asi con las flags  -fopt-info-vec-missed  y -fopt-info-vec-note podemos ver que absolutamente ningun loops es autovectorizado por gcc.

##### Clang:

La vectorizacion esta habilitada por defecto en clang. 
 
Algunas de las flags mas importantes son:

    --fno-vectorize

desabilita la autovectorizacion.

    -Rpass=loop-vectorize

identifica los loops que fueron exitozamente vectorizados.

    -Rpass-missed=loop-vectorize

identifica los loops que no fueron exitozamente vectorizados.

    -Rpass-analysis=loop-vectorize

identifica porque fallo una vectorizacion 

    -Rpass-analysis=loop-vectorize-fsave-optimization-record

 enumera múltiples causas de falla de vectorización.

con clang obtenemos un poco mas de informacion de porque no esta pudiendo vectorizar ya que la flag -Rpass-analysis=loop-vectorize nos dice que la funcion rand() que genera un numero random no puede ser vectorizada y por otro lado no puede determinar el numero de loops en el do while.

#### Vectorizacion:
