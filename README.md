# DECIDERATOR Rhythm Engine v0.2.0

## Nuevo en 0.2.0

- **SPACE / FX** alterna la fila inferior con reverb, stutter y lo-fi.
- Reverb FDN estéreo adaptada de **Aquaton Reverb por aquanode**, con tamaño,
  decay, brillo y modulación; ocho líneas fijas y buffers preasignados.
- Stutter de audio anterior en los pasos 02/04/06/etc., con slices a 1/16, 1/32 o
  1/64 del tempo del host. Lo-fi digital: mezcla, 4-16 bits y sample hold 1-32.
- **GUARDAR / CARGAR**, selector de presets y botones anterior/siguiente dentro
  del plugin. Archivos `.decipreset` XML con validación y escritura temporal.
- **CREDITOS** abre los avisos y el texto completo de las licencias incorporadas.
- Se conservan el zoom 75-150 %, el tamaño compacto y la identidad VST3.

Los controles nuevos arrancan a cero al recuperar estados de versiones anteriores.
Las escenas de 0.2.0 incluyen los efectos nuevos; recuperar un FLP antiguo no aplica
una escena automáticamente. El logotipo y el rediseño visual quedan para otra etapa.

Efecto creativo VST3 para Windows x64. JUCE 8.0.12, C++17 y CMake.
Dos secuencias editables de 16 pasos controlan volumen y filtro; una cadena de
saturación, eco ping-pong y balance estéreo transforma pads, voces, loops e instrumentos.
Es un efecto: necesita recibir audio. No genera batería ni sonido por sí solo.

## Crear algo en un minuto

La interfaz ahora abre en 800 x 496, con selector de zoom 75/80/90/100/125/150 %.
Puedes arrastrar la esquina inferior derecha para ajustar libremente la ventana;
los controles conservan sus proporciones. El tamaño se guarda con el estado del
plugin en el proyecto y también se conserva al cerrar/reabrir el editor.
El zoom afecta a la vista, no cambia el sonido ni los parámetros.

Reproduce una pista a través del plugin y pulsa PULSE, SHARDS o DRIFT. Edita las
barras y ajusta DRY/WET. GENERATE crea otras variaciones; UNDO/REDO recupera el
estado anterior a la última generación o escena. NEUTRAL desactiva los efectos.

- **Pulse:** patrón de volumen con transiciones suavizadas de 3 ms.
- **Tone:** patrón de filtro paso bajo, profundidad y frecuencia máxima.
- **Heat:** saturación suave.
- **Echo:** retardo ping-pong sincronizado, tres tiempos y feedback limitado a 0.7.
- **Orbit:** balance estéreo cíclico durante el patrón.
- **Dry/Wet y Output:** mezcla global y ganancia final de -60 a +12 dB.
- Sincronización a BPM/PPQ del host, seguimiento de play/stop/seek/loop y reloj
  de respaldo. Todos los controles y pasos se guardan como parámetros.

El estado inicial es neutro para conservar la función de ganancia de v0.0.1.
Las escenas activan los efectos. El generador conserva el nivel de salida y el tempo.
Consulta [CREATIVE-GUIDE.md](docs/CREATIVE-GUIDE.md) para el comportamiento exacto.

## Instalar o actualizar en FL Studio

1. Guarda el proyecto y cierra FL Studio antes de sustituir el plugin.
2. Descomprime el paquete **Windows-x64**, no el ZIP de fuentes.
3. Copia la carpeta completa `DECIDERATOR Rhythm Engine.vst3` a
   `C:\Program Files\Common Files\VST3`. Conserva una copia del bundle antiguo.
4. Abre FL Studio > Options > Manage plugins > Find installed plugins. Carga el
   efecto en un slot del Mixer y verifica que el panel indica **0.2.0**.

El bundle contiene `Contents/x86_64-win/DECIDERATOR Rhythm Engine.vst3` y los recursos;
no copies únicamente el archivo interno. No tiene un ejecutable independiente.
Se conservan identidad VST3 y parámetro gainDb. Los estados de v0.0.1 se migran con
efectos nuevos neutros. No guardar un proyecto de v0.1.0 y abrirlo en v0.0.1 esperando
que esa versión antigua conserve parámetros que no conoce.

## Compilar

Requisitos: Git, CMake >=3.22, Visual Studio con herramientas C++ x64 y Windows SDK.
Con Visual Studio 2022:

~~~powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
~~~

Alternativa, desde x64 Native Tools Command Prompt de Visual Studio 2026:

~~~bat
cmake -S . -B build-nmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-nmake
ctest --test-dir build-nmake --output-on-failure
~~~

Resultado: `build/Deciderator_artefacts/Release/VST3/DECIDERATOR Rhythm Engine.vst3`
(o bajo build-nmake). La configuración obtiene JUCE fijado al commit
`29396c22c93392d6738e021b83196283d6e4d850`. Para usar un checkout local idéntico,
añadir `-DFETCHCONTENT_SOURCE_DIR_JUCE=RUTA_ABSOLUTA_A_JUCE`.

El ejecutable DecideratorValidation admite `--preview RUTA_ABSOLUTA.png 80 fx` para renderizar una
captura del editor. Las pruebas del procesador no sustituyen un test del host.

## Alcance y limitaciones

Estéreo, sin MIDI ni latencia añadida. Se declara una cola máxima de 30 s.
Las escenas no son transacciones atómicas de todos los parámetros. Automatización
por bloque con suavizado de 20 ms; no se promete precisión VST3 por muestra.
El estado no incluye el contenido del retardo ni la fase del reloj de respaldo.
No hay limitador, sobremuestreo, secuenciador de batería ni matriz de rutas.
HEAT puede generar aliasing y aumentar el nivel. Vigilar la ganancia al experimentar.

La idea general de un multiefecto rítmico se tomó como referencia de Tantra, pero
esta implementación tiene sus propios algoritmos, escenas, diseño y alcance.
No se usaron código, presets, gráficos ni otros recursos de Tantra. No existe afiliación.
La reverb sí adapta código AGPL de Aquaton, con créditos y originales en third_party/aquanode.

## GitHub, licencia y verificaciones

El repositorio local incluye workflow Windows x64: compilar, ejecutar CTest y
empaquetar el bundle con fuentes propias, JUCE y licencias. La ejecución de ese
workflow en GitHub se documenta por separado de la compilación local.

Repositorio público: https://github.com/deciderator/deciderator-rhythm-engine

Para obtener las fuentes:

~~~powershell
git clone https://github.com/deciderator/deciderator-rhythm-engine.git
cd deciderator-rhythm-engine
~~~

No guardar credenciales en archivos del proyecto.

AGPL-3.0-only; ver LICENSE y THIRD_PARTY_NOTICES.md. JUCE se usa bajo su opción AGPL.
Las dependencias conservan sus propias licencias. El SDK VST3 está incluido en JUCE.
Al redistribuir un binario hay que proporcionar su código correspondiente y avisos.

Resultados ejecutados y pendientes: [VERIFICATION.md](docs/VERIFICATION.md).
Matriz de pruebas en FL Studio: [TESTING.md](docs/TESTING.md).
