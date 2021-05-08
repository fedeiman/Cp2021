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
