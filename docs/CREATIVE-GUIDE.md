# Time Lab y edición en ventanas — v0.3.0

TIME LAB abre una ventana independiente y ampliable. FX WINDOW abre los efectos con controles grandes; ambas pueden coexistir con la principal. Cerrar la principal destruye sus ventanas auxiliares. Sus posiciones no se guardan.

Para escuchar el nuevo motor aislado: NEUTRAL, TIME LAB, SCRATCH o REVERSE, TIME MIX 100%. Reproduce unos ciclos para llenar BUFFER. Arrastra la curva naranja de 33 puntos; Shift permite ajuste fino y el campo PUNTO admite números. SNAP ajusta verticalmente a la rejilla; LINEA interpola y STEPS hace saltos. Doble clic o botón derecho pone el punto a directo.

El eje horizontal es tiempo musical; el vertical indica retardo hacia audio anterior (hasta dos ciclos). La onda azul muestra la envolvente de entrada reciente. No es un EQ ni edición destructiva del audio. Cambiar la velocidad modifica también la afinación. CHOP 16 repite fragmentos; TAPE STOP frena; SCRATCH alterna velocidad y dirección. DIRECTO desactiva Time Mix. Los presets guardan la curva, pero no el audio capturado.
# Crear movimiento con DECIDERATOR 0.2.0

## Reverb, stutter y lo-fi

Pulsa SPACE / FX para ver los controles nuevos en la fila inferior. RHYTHM / FX
vuelve a la fila de ganancia, mezcla y efectos previos. Cambiar de página no
desactiva efectos: modifica SPACE, STUTTER y LO-FI a 0 para omitir su mezcla.

- **SPACE** mezcla la reverb adaptada de Aquaton; SIZE cambia los retardos,
  DECAY la realimentación, AIR el brillo y SPACE MOD la modulación de retardo.
  SPACE MOD no es un pitch-shifter de octava. No hay freeze infinito.
- **STUTTER** repite un fragmento anterior durante los pasos 02/04/06/etc. SLICE
  elige semicorcheas (1/16), fusas (1/32) o semifusas (1/64). Con tempo
  120 BPM, 1/16 son 125 ms; 1/32 son 62.5 ms; 1/64 son 31.25 ms.
  Si todavía no hay suficiente audio capturado, conserva la señal de entrada.
- **LO-FI** mezcla cuantización BITS (4-16) y retención HOLD (1-32 muestras).
  Es degradación digital deliberada y puede producir aliasing.

Cadena: Pulse > Tone > Heat > Stutter/Lo-fi > Echo > Space > Orbit > Dry/Wet > Output.
No hay limitador de salida. Empezar a volumen moderado y usar OUTPUT para compensar.

## Presets propios

GUARDAR abre un diálogo para escribir un archivo con extensión `.decipreset`.
La carpeta propuesta es `%APPDATA%\DECIDERATOR\Presets`; el selector lista los
archivos de esa carpeta y las cuatro escenas de fábrica. CARGAR permite elegir
un preset en otra ubicación. Usa < y > para recorrer los elementos del selector.

El preset contiene todos los parámetros y los 32 pasos; no incluye audio del
buffer, colas, posición de reproducción ni el tamaño de ventana. Cargar un preset
conserva el tamaño de ventana actual. El proyecto del host sí guarda ese tamaño.
Los errores de formato o valores fuera de rango se rechazan antes de aplicar
el estado; un archivo guardado con éxito sustituye el destino mediante un temporal.

CREDITOS muestra la licencia AGPL y la atribución de Aquaton dentro del plugin.

## Ajustar el tamaño

El selector superior VISTA / ZOOM cambia toda la interfaz entre 75 y 150 %.
El inicio es 80 % (800 x 496), adecuado para una ventana más compacta.
También puedes arrastrar la esquina inferior derecha. Las proporciones se
conservan y puede quedar un margen si eliges una forma de ventana diferente.
El tamaño elegido se guarda dentro del proyecto, no como parámetro de audio.

La apariencia se conserva de forma provisional: esta etapa prioriza funcionalidad.

Es un efecto: necesita recibir audio. Inserta el plugin en el Mixer de FL Studio
y reproduce un pad, una voz, un loop o un instrumento sostenido.

## Primer minuto

1. Baja el volumen de escucha y pulsa **01 / PULSE**. Crea cortes rítmicos.
2. Prueba **02 / SHARDS** para acentos irregulares y más textura.
3. Prueba **03 / DRIFT** para filtro, espacio y movimiento más suave.
4. Pulsa **GENERATE** para obtener otra combinación; **UNDO / REDO** alterna con
   el estado anterior a la última escena o generación.
5. Ajusta **DRY / WET** para conservar parte de la señal original.

## Dos patrones, una interpretación

Cada barra es un paso, de 0 a 1. Arrastra verticalmente para editarla. Doble clic:
valor máximo. Los 16 pasos se repiten; STEP selecciona la duración de cada paso.
En 1/16, el ciclo dura cuatro negras: un compás de 4/4. En 1/8 dura ocho negras.
El ciclo está basado en negras, no cambia de longitud según la firma de compás.

- **PULSE:** patrón de volumen. Con profundidad 0 no corta; con 1 llega al valor
  dibujado, incluido silencio. Las transiciones se suavizan unos 3 ms.
- **TONE:** patrón de apertura de un filtro paso bajo. CEILING determina su límite
  superior; las barras bajas lo oscurecen. TONE a 0 desactiva el filtro.
- **HEAT:** saturación suave. Puede aumentar el nivel y generar aliasing; esta
  primera versión no tiene sobremuestreo.
- **ECHO:** cantidad de eco ping-pong. ECHO TIME selecciona 1/8, 1/8 con puntillo
  o 1/4. FEEDBACK controla las repeticiones, hasta 0.7.
- **ORBIT:** balance estéreo cíclico a lo largo de los 16 pasos. No convierte una
  fuente de un solo canal en una fuente centrada: atenúa alternativamente L/R.
- **OUTPUT:** ganancia final de -60 a +12 dB. No hay limitador automático.

## Reloj y automatización

Usa BPM y PPQ del host cuando están disponibles. El indicador HOST SYNC confirma
que se recibió posición PPQ; en FREE CLOCK se usa el reloj interno, con el BPM del
host si existe o el BPM de respaldo del panel. Con información de transporte,
el patrón sigue play/stop y salta al paso correspondiente tras un seek o loop.
Al detener el host, el patrón se congela pero las colas de eco pueden continuar.

Los controles y los 32 pasos son parámetros automatizables y se guardan en el
proyecto. La automatización se lee por bloque y se suaviza; no se promete
automatización VST3 precisa por muestra. Las escenas cambian varios parámetros
con gestos independientes: no se garantiza un cambio de escena atómico.

GENERATE conserva OUTPUT, BPM, STEP, ECHO TIME, CEILING, FEEDBACK y DRY/WET;
modifica los patrones y cantidades de efectos dentro de rangos acotados.
UNDO / REDO no es un historial general de movimientos de controles.
Las colas de audio y la posición interna no se guardan como parte del preset.

## Actualizar desde 0.0.1

Cierra FL Studio, guarda una copia del bundle antiguo y sustituye la carpeta
completa `DECIDERATOR Rhythm Engine.vst3` por la de 0.2.0. No copies las fuentes
ni únicamente el binario interno. Reescanea plugins y verifica la versión en el panel.

Se conservan la identidad VST3 y el ID gainDb. Al cargar un estado antiguo que
sólo tenía ganancia, los efectos nuevos quedan neutros. La migración se comprueba
en pruebas del procesador; confirma también la recuperación de tus FLP reales.
