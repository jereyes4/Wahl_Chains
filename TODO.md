# Cosas implementadas / TODO

## Input (Reader.cpp)
- [x] Reconocer opciones.
- [x] Reconocer fibras / secciones.
- [x] Reconocer y recordar singularidades / blowups.
- [x] Armar grafo.
  
## Pretest (Searcher.cpp)
- [x] Recuperar pretests del input.
- [x] Construir grafo de pretest.
  - [x] Blowdowns.
    - [x] Optimizar: evitar repeticiones.
- [x] Cálculo de invariantes.
- [x] Chequeo de obstrucción.

## Análisis del grafo (Graph.cpp)
- [x] Blowups / blowdowns.
- [x] Exploración completa de separaciones del grafo.
- [x] Exploración completa de separaciones del grafo incluyendo marco central.
- [x] Extracción de cadenas.
- [x] Extracción de ciclos.
- [x] Extracción de tenedores.

## 1 cadena (single_chain.cpp)
- [x] Filtrar separaciones del grafo y extraer cadenas.
  - [x] Optimizar: evitar repeticiones.
- [x] Testeo de blowups extra + contracciones.
- [x] Verificación de Wahl + cálculo de invariantes.
- [x] Chequeo nef.
- [x] Chequeo Q-ef.

## 2 cadenas (double_chain.cpp)
- [x] Filtrar separacinoes del grafo y extraer cadenas.
  - [x] Optimizar: evitar repeticiones (fácil).
  - [ ] Optimizar: evitar repeticiones (dificil).
- ### No resolución parcial:
  - #### Testeo de blowups extra + contracciones
    - [x] Caso dos cadenas independientes.
    - [x] Caso dos cadenas, una independiente.
    - [x] Caso dos cadenas mutuamente dependientes.
  - [x] Verificación de Wahl + cálculo de invariantes.
- ### Resolución parcial
  - [x] Blowups extra + contracciones.
  - [x] Existencia y recuperación de resoluciones parciales.
  - [x] Cálculo de invariantes.
  - [x] Verificación de Agujeros de Gusano + posibles contraejemplos. 
- [x] Chequeo nef.
- [x] Chequeo Q-ef.

## 1 QHD (QHD_single_chain.cpp + single_chain.cpp)
- [x] Filtrar separaciones del grafo y extraer tenedores o cadena.
  - [x] Optimizar: evitar repeticiones.
- [x] Posible caso con extra blowups a partir de tenedor cíclico.
- [x] Posible caso con extra blowups a partir de una cadena.
- [x] Contracciones.
- [x] Verificación QHD y cálculo de invariantes.
- [x] Chequeo nef.
- [x] Chequeo Q-ef.

## 1 QHD, 1 cadena (QHD_double_chain.cpp + double_chain.cpp)
- [x] Filtrar separaciones del grafo y extraer tenedores y cadenas.
  - [x] Caso general.
  - [x] Posible caso con extra blowups sobre el tenedor a partir de tenedor cícliclo.
  - [x] Posible caso con extra blowups sobre el tenedor a partir de una o dos cadenas.
  - [x] Optimizar: evitar repeteciones (fácil).
  - [ ] Optimizar: evitar repeteciones (dificil).
- ### No resolución parcial:
  - [x] Testeo de blowups extra sobre la cadena.
  - [x] Verificación QHD + Wahl + cálculo de invariantes.
- ### Resolución parcial:
  - ### Caso separación en un blowup.
    - [ ] Testear todas las separaciones en un blowup.
    - [ ] Compensar con blowups extra sobre la cadena.
    - [ ] Rehabilitar pruning que ignora casos de no resolución parcial que se reducen a resolución parcial.
  - ### Caso separación en más de un blowup.
    - [ ] Recuperar todas las posibilidades para el tenedor a partir de la información en otras ramas.
      - [ ] Tipo a.
      - [ ] Tipo b.
      - [ ] Tipo c.
      - [ ] Tipo d.
      - [ ] Tipo e.
      - [ ] Tipo f.
      - [ ] Tipo g.
      - [ ] Tipo h.
      - [ ] Tipo i.
      - [ ] Tipo j.
  - [ ] Verificación QHD + Wahl + cálculo de invariantes.
- [x] Chequeo nef.
- [x] Chequeo Q-ef.

## Output (Wahl.cpp + Writer.hpp)
- [x] Juntar y ordenar ejemplos.
  - [x] 1 cadena.
  - [x] 2 cadenas.
  - [x] 1 QHD.
  - [x] 1 QHD + 1 cadena.
- ### Exportar json
  - [x] Información global.
  - [x] 1 cadena.
  - [x] 2 cadenas.
  - [x] 1 QHD.
  - [x] 1 QHD + 1 cadena.
- ### Exportar txt
  - [x] 1 cadena.
  - [x] 2 cadenas.
  - [x] 1 QHD.
  - [x] 1 QHD + 1 cadena.
- ### Exportar latex
  - [x] 1 cadena.
  - [x] 2 cadenas.
  - [x] 1 QHD.
  - [x] 1 QHD + 1 cadena.

## Algoritmos extra (Algorithms.hpp)
- [x] Estructura opcional para optimizaciones.
- [x] Fracciones continuas + verificación Wahl.
- [x] Cálculo de discrepancias Wahl.
- [x] Contracciones.
- [x] Estructura para blowups / blowdowns + recordar orden y nombres.

## Algoritmos extra para QHD (QHD_functions.hpp)
- ### Recuperar tipo de QHD
  - [x] Tipo a.
  - [x] Tipo b.
  - [x] Tipo c.
  - [x] Tipo d.
  - [x] Tipo e.
  - [x] Tipo f.
  - [x] Tipo g.
  - [x] Tipo h.
  - [x] Tipo i.
  - [x] Tipo j.
- ### Cálculo de discrepancias.
  - [x] Tipo a.
  - [x] Tipo b.
  - [x] Tipo c.
  - [x] Tipo d.
  - [x] Tipo e.
  - [x] Tipo f.
  - [x] Tipo g.
  - [x] Tipo h.
  - [x] Tipo i.
  - [x] Tipo j.
  
## Decodificación json (Display<span>.</span>py)
- [ ] Información global.
- [x] 1 cadena.
- [x] 2 cadenas.
- [x] 2 cadenas - resolución extremal.
- [x] 1 QHD.
- [x] 1 QHD + 1 cadena.
- [ ] 1 QHD + 1 cadena - resolución parcial.
