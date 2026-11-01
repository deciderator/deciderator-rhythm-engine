# Cambios

## 0.3.0

- Time Warp causal con 33 puntos, lectura estéreo interpolada y crossfade de saltos.
- Plantillas de chop/reversa/tape stop/scratch, mezcla y ciclos sincronizados.
- Time Lab independiente con onda reciente, rejilla, dibujo y ajuste numérico.
- Inspector de efectos independiente, redimensionable y enlazado a los parámetros.
- Compatibilidad de estados/presets anteriores con Time Warp apagado.
- Estimación de cola dinámica para contemplar los retardos de Time Warp.

## 0.2.0

- SpaceFDN, adaptación AGPL acreditada de Aquaton por aquanode: ocho líneas,
  decay acotado, brillo y modulación. Fuentes originales conservadas.
- Stutter sincronizado sobre audio anterior y lo-fi de bits/sample hold.
- Página funcional SPACE / FX dentro de la ventana existente.
- Presets .decipreset: guardar, cargar, selector y navegación; validación de datos.
- Créditos/licencias incorporados y accesibles desde el plugin.
- Dependencias de cabeceras propias explícitas para compilación incremental.

## 0.1.1

- Interfaz escalable con selector 75/80/90/100/125/150 %.
- Inicio compacto a 800 x 496 y redimensionado libre desde la esquina.
- Conservación de proporciones y de tamaño al reabrir el editor o recuperar el estado.
- La arquitectura del editor separa la vista de controles del marco redimensionable.
- Se mantienen el motor de audio y todos los parámetros de v0.1.0.
- Pruebas de geometría de la vista y persistencia de tamaño añadidas a CTest.

## 0.1.0

Dos patrones de 16 pasos para volumen/filtro, saturación, eco ping-pong,
balance estéreo, escenas, generador y deshacer. Sincronización con el host.

## 0.0.1

Ganancia estéreo, interfaz inicial y persistencia de ganancia.

## Siguiente prioridad

Pruebas musicales en FL Studio, rendimiento real, mejoras de sonido y después
rediseño visual y logotipo. No se incluyen esos cambios gráficos en 0.2.0.
